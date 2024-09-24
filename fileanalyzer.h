#ifndef FILEANALYZER_H
#define FILEANALYZER_H

#include <QMainWindow>
#include <QTreeView>
#include <QChartView>
#include <QPushButton>
#include <QLabel>
#include <QStatusBar>
#include <QProgressBar>
#include <QFileSystemModel>

#include "filedetailswidget.h"
#include "contextmenuhandler.h"


QT_BEGIN_NAMESPACE
namespace Ui { class FileAnalyzer; }
QT_END_NAMESPACE

// 添加对核心工具类的声明
class FileScanner;
class FileSystemHandler;
class ChartHandler;

/**
 * @brief The FileAnalyzer class
 * 作为协调者，管理 UI 和核心组件之间的交互。
 * 具体的核心功能委派给各个相关的类：
 *      1. 文件系统、扫描逻辑、图表更新
 */
class FileAnalyzer : public QMainWindow
{
    Q_OBJECT

public:
    FileAnalyzer(QWidget *parent = nullptr);
    ~FileAnalyzer();

private slots:

    void onAnalyzeClicked();
    void onDeleteClicked();
    void onMoveClicked();
    void onFileSelected(const QItemSelection &selected, const QItemSelection &deselected);

    // FileScannner slots
    void onScanClicked();
    void onScanProgress(int percentage);
    void onScanCompleted();
//    void onFolderChanged(const QString &path);
    // ContextMenu slots
    void showContextMenu(const QPoint& pos);
    void onOpenInExplorerRequested();

private:
    Ui::FileAnalyzer *ui;

    // Core components
    FileScanner *fileScanner;       // fileScanner 文件扫描
    FileSystemHandler *fileSystemHandler;
    QProgressBar *progressBar;      // 进度条
    ChartHandler *chartHandler;

    // UI elements
    QTreeView *fileTreeView;
//    QChartView *chartView;
    QLabel *scanStatusLabel;
//    QLabel *detailsLabel;
    // Buttons
    QPushButton *scanButton;
    QPushButton *analyzeButton;
    QPushButton *deleteButton;
    QPushButton *moveButton;
    QPushButton *backButton;

    QStatusBar *statusBar;
    FileDetailsWidget *fileDetailsWidget;

    QString currentScanPath;
    // 右键点击菜单
    ContextMenuHandler *contextMenuHandler;
    QModelIndex currentContextMenuIndex;    // 存储右键点击的项的索引
    QString currentContextMenuFilePath;

    void setupUi();
    void updateFileDetails(const QString& path);
    void analyzeDirectory(const QString &path);
    void deleteFile(const QString &path);
    void updatePieChart(const QString& path);
    void onFolderChanged(const QString &path);

};
#endif // FILEANALYZER_H
