#include "fileanalyzer.h"
#include "./ui_fileanalyzer.h"
#include "layeredfilescanner.h"
#include "FileSystemHandler.h"
#include "charthandler.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QtCharts>
#include <QDebug>

/**
 * @brief FileAnalyzer::FileAnalyzer
 * 在构造函数中对 核心工具类 fileSystemHandler 进行实例化
 * @param parent
 */
FileAnalyzer::FileAnalyzer(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::FileAnalyzer)
    , fileSystemHandler(new FileSystemHandler(this))
    , fileScanner(new LayeredFileScanner(this))
    , chartHandler(new ChartHandler(this, fileScanner))
    , fileDetailsWidget(new FileDetailsWidget(this))
    , contextMenuHandler(new ContextMenuHandler(this))
{
    ui->setupUi(this);
    setupUi();
    this->setStyleSheet(qApp->styleSheet());

    // 为文件树视图和文件详情部件启用自定义上下文菜单
    fileTreeView->setContextMenuPolicy(Qt::CustomContextMenu);
    fileDetailsWidget->setContextMenuPolicy(Qt::CustomContextMenu);

    // 连接FileScanner的信号和槽
    connect(fileScanner, &FileScanner::progressUpdated, this, &FileAnalyzer::onScanProgress);
    connect(fileScanner, &FileScanner::scanFinished, this, &FileAnalyzer::onScanCompleted);
    connect(chartHandler, &ChartHandler::folderChanged, this, &FileAnalyzer::onFolderChanged);
    connect(backButton, &QPushButton::clicked, chartHandler, &ChartHandler::goToParentFolder);
    // 连接ContextMenu的信号和槽
    // 1. 连接自定义上下文菜单信号到 showContextMenu槽
    connect(fileTreeView, &QTreeView::customContextMenuRequested, this, &FileAnalyzer::showContextMenu);
    connect(fileDetailsWidget, &FileDetailsWidget::customContextMenuRequested, this, &FileAnalyzer::showContextMenu);
    // 2. 连接ContextMenuHandler 的 openInExplorerRequest 信号到槽
    connect(contextMenuHandler, &ContextMenuHandler::openInExplorerRequested,
            this, &FileAnalyzer::onOpenInExplorerRequested);
}

FileAnalyzer::~FileAnalyzer()
{
    delete ui;
}

void FileAnalyzer::setupUi()
{
    setWindowTitle("File Analyzer");

// Central widget and main layout
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    setCentralWidget(centralWidget);

    // Toolbar
    QHBoxLayout *toolbarLayout = new QHBoxLayout();
    toolbarLayout->setSpacing(10);
    toolbarLayout->setContentsMargins(10, 10, 10, 10);

    scanButton = new QPushButton("Scan", this);
    analyzeButton = new QPushButton("Analyze", this);
    deleteButton = new QPushButton("Delete", this);
    moveButton = new QPushButton("Move", this);
    backButton = new QPushButton("Back", this);


    toolbarLayout->addWidget(scanButton);
    toolbarLayout->addWidget(analyzeButton);
    toolbarLayout->addWidget(deleteButton);
    toolbarLayout->addWidget(moveButton);
    toolbarLayout->addWidget(backButton);
    toolbarLayout->addStretch();

    mainLayout->addLayout(toolbarLayout);

// Main content area
    QHBoxLayout *contentLayout = new QHBoxLayout();
    contentLayout->setSpacing(20);
    contentLayout->setContentsMargins(10, 0, 10, 10);

//-------------- 1. File tree view setup--------------------
    fileTreeView = new QTreeView(this);   
    fileTreeView->setColumnWidth(0, 250);
    fileTreeView->setAnimated(false);
    fileTreeView->setIndentation(20);
    fileTreeView->setSortingEnabled(true);
    fileTreeView->setSelectionMode(QAbstractItemView::SingleSelection);   //set one item can be selected at a time
    fileTreeView->setEditTriggers(QAbstractItemView::NoEditTriggers);  // Ban Edite function
    fileTreeView->setExpandsOnDoubleClick(true);    //double click can expand tree node

    // Obtain the file system model from FileSystemHandler
    QFileSystemModel* model = fileSystemHandler->getFileSystemModel();
    fileTreeView->setModel(model);

    fileTreeView->setRootIndex(model->index(QDir::rootPath()));
//    fileTreeView->setColumnHidden(1, true);
//    fileTreeView->setColumnHidden(2, true);
//    fileTreeView->setColumnHidden(3, false);
    fileTreeView->setRootIndex(model->index(""));
    // Sort by the first column (name) in ascending order to display drives alphabetically
    fileTreeView->sortByColumn(0, Qt::AscendingOrder);
    // Add the file tree view to the layout
    contentLayout->addWidget(fileTreeView, 1);
// ---------------File tree view end---------------------


//----------------2.Right side (chart and details)------------------
    QVBoxLayout *rightLayout = new QVBoxLayout();

    // 2.1初始化进度条
    progressBar = new QProgressBar(this);
    progressBar->setRange(0, 100);
    progressBar->setValue(0);
    progressBar->setTextVisible(true);

    // 2.2File Distribution Chart
    QChartView *chartView = chartHandler->getChartView();
    // 初始化饼状图
    chartView->setRenderHint(QPainter::Antialiasing);
    QPieSeries *series = new QPieSeries();
    series->append("Documents", 1);
    series->append("Images", 1);
    series->append("Videos", 1);
    series->append("Others", 1);
    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("File Distribution");
    chartView->setChart(chart);

    rightLayout->addWidget(progressBar);
    rightLayout->addWidget(chartView, 2);

    // 3.Details panel
//    detailsLabel = new QLabel("File Details", this);
    rightLayout->addWidget(fileDetailsWidget, 1);

    contentLayout->addLayout(rightLayout, 1);
    mainLayout->addLayout(contentLayout);

    // Status bar
    statusBar = new QStatusBar(this);
    setStatusBar(statusBar);
    statusBar->addPermanentWidget(new QLabel("Items: 0"));
    statusBar->addPermanentWidget(new QLabel("Size: 0 KB"));

    // Connect signals and slots
    connect(scanButton, &QPushButton::clicked, this, &FileAnalyzer::onScanClicked);
    connect(analyzeButton, &QPushButton::clicked, this, &FileAnalyzer::onAnalyzeClicked);
    connect(deleteButton, &QPushButton::clicked, this, &FileAnalyzer::onDeleteClicked);
    connect(moveButton, &QPushButton::clicked, this, &FileAnalyzer::onMoveClicked);
    connect(fileTreeView->selectionModel(), &QItemSelectionModel::selectionChanged, this,
            &FileAnalyzer::onFileSelected);
    // 连接chart和fileDetails的信号和槽
    connect(chartHandler, &ChartHandler::directoryDataUpdated, fileDetailsWidget,
            &FileDetailsWidget::updateDetails);
}

/**
 * @brief FileAnalyzer::onScanClicked
 * 执行文件扫描，发生点击事件后，将开始扫描某目录下的文件。
 *  ·扫描过程中，显示扫描进度条
 *  ·扫描结束后，用饼状图显示磁盘存储状况
 */
void FileAnalyzer::onScanClicked()
{
    if (!fileTreeView) {
        qDebug() << "Error: fileTreeView is not initialized!";
        return;
    }

    QModelIndex selectedIndex = fileTreeView->currentIndex();

    if(selectedIndex.isValid()){
        // 如果有选中的文件夹，则获取其路径
        currentScanPath  = fileSystemHandler->getSelectedPath(selectedIndex);
    }

    // 如果没有选中的文件夹，则使用根目录
    if(currentScanPath .isEmpty()){
        currentScanPath  = "";     // 设置为根目录
    }

    // 重置进度条
    progressBar->setValue(0);
    // 开始扫描
    fileScanner->startScan(currentScanPath);

}

void FileAnalyzer::onAnalyzeClicked()
{

}

void FileAnalyzer::onDeleteClicked()
{

}

void FileAnalyzer::onMoveClicked()
{

}

void FileAnalyzer::onFileSelected(const QItemSelection &selected, const QItemSelection &deselected)
{

}

void FileAnalyzer::analyzeDirectory(const QString &path)
{

}

/***********************************************************
 *                功能: 文件读取                   *
 ***********************************************************/
// 处理扫描进度更新
void FileAnalyzer::onScanProgress(int percentage)
{
    qDebug() << "Scan completed.";
    progressBar->setValue(percentage);  // 更新进度条
}

// chart
void FileAnalyzer::updatePieChart(const QString& path)
{
    chartHandler->updatePieChart(path);
}

// 处理扫描完成
void FileAnalyzer::onScanCompleted()
{
    qDebug() << "Scan completed.";
    updatePieChart(currentScanPath);  // 更新饼状图显示
}

void FileAnalyzer::onFolderChanged(const QString &path)
{
    // 更新文件树的选择
    QModelIndex index = fileSystemHandler->getFileSystemModel()->index(path);
    fileTreeView->setCurrentIndex(index);

    // 更新图表
    chartHandler->updatePieChart(path);

    // FileDetailsWidget 会通过 ChartHandler 的信号自动更新
}

/***********************************************************
 *                功能: 右键菜单栏                   *
 ***********************************************************/
/**
 * @brief FileAnalyzer::showContextMenu
 * @param pos
 * 在主界面显示菜单
 */
void FileAnalyzer::showContextMenu(const QPoint& pos)
{
    QObject* sender = QObject::sender(); // 返回指向发射信号的对象指针

    if(sender == fileTreeView){
        QModelIndex index = fileTreeView->indexAt(pos);
        if(index.isValid()){
            currentContextMenuIndex = index;
            contextMenuHandler->showContextMenu(fileTreeView->viewport()->mapToGlobal(pos));
        }
    }
    else if(sender == fileDetailsWidget){
        QString filePath = fileDetailsWidget->getSelectedFilePath();
        if(!filePath.isEmpty()){
            currentContextMenuFilePath = filePath;
            contextMenuHandler->showContextMenu(pos);
        }
    }
}

/**
 * @brief onOpenInExplorerRequested
 * 函数响应 ContextMenuHandler 发出的 openInExplorerRequested 信号。
 * 当用户在文件树视图或文件详情表格中右键点击并选择"Open in Explorer"选项时，
 * 该信号会被触发。
 * 功能：
 * 1. 确定要在资源管理器中打开的文件或文件夹路径。
 *    - 如果是从文件树视图触发，使用 currentContextMenuIndex。
 *    - 如果是从文件详情表格触发，使用 currentContextMenuFilePath。
 * 2. 使用系统默认的文件资源管理器打开包含选中文件的文件夹。
 */
void FileAnalyzer::onOpenInExplorerRequested()
{
    QString path;
    if(currentContextMenuIndex.isValid()){
        path = fileSystemHandler->getSelectedPath(currentContextMenuIndex);
    }else if(!currentContextMenuFilePath.isEmpty()){
        path = currentContextMenuFilePath;
    }

    if(!path.isEmpty()){
        QDesktopServices::openUrl(QUrl::fromLocalFile(QFileInfo(path).dir().absolutePath()));
    }
    // 重置上下文菜单相关的成员变量
    currentContextMenuIndex = QModelIndex();
    currentContextMenuFilePath.clear();
}
