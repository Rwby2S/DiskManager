#include "filedetailswidget.h"
#include "ui_filedetailswidget.h"
#include <QVBoxLayout>
#include <QHeaderView>
#include <QJsonObject>
#include <QJsonArray>
#include <QFileInfo>
#include <QFile>

FileDetailsWidget::FileDetailsWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FileDetailsWidget)
{
    ui->setupUi(this);
    setupUI();
    loadStyleSheet();
}

FileDetailsWidget::~FileDetailsWidget()
{
    if(titleLabel != nullptr)
        delete titleLabel;
    if(detailsTable != nullptr)
        delete detailsTable;
    delete ui;
}

void FileDetailsWidget::setupUI()
{
    QVBoxLayout *layout = new QVBoxLayout(this);

    titleLabel = new QLabel("File Details", this);
    titleLabel->setObjectName("titleLabel");
    layout->addWidget(titleLabel);

    detailsTable = new QTableWidget(this);
    detailsTable->setObjectName("detailsTable");
    detailsTable->setColumnCount(3);
    detailsTable->setHorizontalHeaderLabels({"Name", "Size", "Modified Date"});
    detailsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    detailsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    detailsTable->setSortingEnabled(true);
    detailsTable->setAlternatingRowColors(true);
    layout->addWidget(detailsTable);

    setLayout(layout);
}

void FileDetailsWidget::updateDetails(const QJsonArray& subItems)
{
    qDebug() << "fileDetailsWidget::updateDetails";
//    QString path = directoryInfo["path"].toString();
//    titleLabel->setText("Contents of: " + path);

//    QJsonArray subItems = directoryInfo["subItems"].toArray();
    detailsTable->setSortingEnabled(false);
    detailsTable->setRowCount(0);

    for(const QJsonValue& value : subItems){
        QJsonObject item = value.toObject();
        int row = detailsTable->rowCount();
        detailsTable->insertRow(row);

        QString name = QFileInfo(item["path"].toString()).fileName();
        qint64 size = item["size"].toString().toLongLong();
        QString sizeStr = formatSize(size);
        QString modifiedDate = item["lastModified"].toString();

        detailsTable->setItem(row, 0, new QTableWidgetItem(name));
        detailsTable->setItem(row, 1, new QTableWidgetItem(sizeStr));
        detailsTable->setItem(row, 2, new QTableWidgetItem(modifiedDate));

        // Store the actual size for sorting
        detailsTable->item(row, 1)->setData(Qt::UserRole, size);
    }

    detailsTable->setSortingEnabled(true);
    detailsTable->sortByColumn(1, Qt::DescendingOrder);
}

QString FileDetailsWidget::formatSize(qint64 size) const
{
    const char* units[] = {"B", "KB", "MB", "GB", "TB"};
    int i = 0;
    double dblSize = size;

    while (dblSize >= 1024 && i < 4) {
        dblSize /= 1024;
        i++;
    }

    return QString("%1 %2").arg(dblSize, 0, 'f', 2).arg(units[i]);
}

void FileDetailsWidget::loadStyleSheet()
{
    QFile file(":/resource/file_details_style.qss");
    if (file.open(QFile::ReadOnly | QFile::Text)) {
        QString styleSheet = QLatin1String(file.readAll());
        this->setStyleSheet(styleSheet);
        file.close();
    }
}
