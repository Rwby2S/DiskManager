#include "charthandler.h"
#include <QtCharts/QChart>
#include <QtCharts/QPieSeries>
#include <QJsonArray>
#include <QJsonObject>
#include <QFileInfo>
#include <QDebug>
#include <QMessageBox>
#include <QDir>


ChartHandler::ChartHandler(QObject *parent, FileScanner* scanner)
    : QObject{parent}, fileScanner(scanner)
{
    chartView = new QChartView(); // 初始化图表视图
    chartView->setRenderHint(QPainter::Antialiasing);

}

ChartHandler::~ChartHandler()
{
    delete chartView;
    if(fileScanner != nullptr)
        delete fileScanner;
}

// 获取图表视图
QChartView* ChartHandler::getChartView() const
{
    return chartView;
}


void ChartHandler::updatePieChart(const QString& currentPath)
{
    curPath = currentPath;

    QJsonObject results = fileScanner->getCachedResults();
    QJsonArray subItems;
    if(currentPath != ""){
        subItems = getSubItemsForPath(results, currentPath);
    }else{
        subItems = results["subItems"].toArray();
    }

    if (subItems.isEmpty()) {
        qDebug() << "Error: No items found in the results.";
        return;
    }

    QPieSeries *series = new QPieSeries();
    QString title = "Disk Usage by Folder: " + QFileInfo(currentPath).fileName();

    for (const QJsonValue &value : subItems) {
        QJsonObject item = value.toObject();
        QString itemPath = item["path"].toString();
        qint64 size = item["size"].toString().toLongLong();
        QFileInfo fileInfo(itemPath);
        QString itemName = fileInfo.fileName();

        // 处理文件夹名称过长的情况
        if (itemName.length() > 15) {
            QFileInfo fileInfo(itemName);
            QString baseName = fileInfo.baseName();
            QString suffix = fileInfo.suffix();

            if (suffix.isEmpty()) {
                // 如果是文件夹或没有后缀的文件
                itemName = baseName.left(12) + "...";
            } else {
                // 对于有后缀的文件
                int maxBaseLength = 9; // 预留3个字符给"..."和至少3个字符给后缀
                if (baseName.length() > maxBaseLength) {
                    itemName = baseName.left(maxBaseLength) + "..." + suffix;
                }
            }
        }

        QPieSlice *slice = series->append(itemName, size);
        slice->setLabelVisible(false);  // 默认隐藏标签

        // 设置鼠标悬浮效果
        setupSliceHoverEffects(slice, itemName, size);

        // 将选中的扇形区域 Connect click 事件
        connect(slice, &QPieSlice::clicked, this, [this, itemPath](){
            handleSliceClicked(itemPath);
        });
    }

    // 创建并设置饼状图
    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle(title);
    chart->legend()->hide();  // 隐藏图例
    chartView->setChart(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->update();  // 更新视图

    qDebug() << "updatePieChart finished";
}

void ChartHandler::setupSliceHoverEffects(QPieSlice* slice, const QString& itemName, qint64 size)
{
    // 获取切片的索引
    QPieSeries* series = qobject_cast<QPieSeries*>(slice->series());
    int sliceIndex = series->slices().indexOf(slice);

    // 使用索引来设置颜色
    QColor sliceColor = generateColorFromIndex(sliceIndex);
    slice->setBrush(sliceColor);

    // 保留原始样式
    QPen originalPen = slice->pen();
    QBrush originalBrush = slice->brush();
    bool originalLabelVisible = slice->isLabelVisible();

    connect(slice, &QPieSlice::hovered, this, [=](bool hover){
        if(hover){
            slice->setExploded(true);
            slice->setLabelVisible(true);
            // 使用原始颜色的亮版本
            QColor lighterColor = generateColorFromIndex(sliceIndex).lighter(50);
            slice->setBrush(lighterColor);
            // 使用原始颜色的深版本作边框
            slice->setPen(QPen(Qt::darkBlue, 2));
            slice->setBrush(slice->brush().color().lighter());

            QString details = QString("<div align='center' style='font-size: 12px; color: #333;'>"
                                      "<b>%1</b><br>"
                                      "<span style='font-size: 10px;'>%2</span>"
                                      "</div>")
                                  .arg(itemName)
                                  .arg(formatSize(size));
            slice->setLabel(details);
        } else{
            // 恢复原始样式
            slice->setExploded(false);
            slice->setLabelVisible(false);
            slice->setPen(QPen(Qt::white, 1));  // 恢复默认边框

            // 恢复原始颜色
            QColor sliceColor = generateColorFromIndex(sliceIndex);
            slice->setBrush(sliceColor);
        }
    });
}

QString ChartHandler::formatSize(qint64 bytes)
{
    const qint64 kb = 1024;
    const qint64 mb = 1024 * kb;
    const qint64 gb = 1024 * mb;
    const qint64 tb = 1024 * gb;

    if (bytes >= tb)
        return QString("%1 TB").arg(QString::number(double(bytes) / tb, 'f', 2));
    if (bytes >= gb)
        return QString("%1 GB").arg(QString::number(double(bytes) / gb, 'f', 2));
    if (bytes >= mb)
        return QString("%1 MB").arg(QString::number(double(bytes) / mb, 'f', 2));
    if (bytes >= kb)
        return QString("%1 KB").arg(QString::number(double(bytes) / kb, 'f', 2));
    return QString("%1 bytes").arg(bytes);
}

QColor ChartHandler::generateColorFromIndex(int index)
{
    return colors[index % colors.size()];
}

QJsonArray ChartHandler::findSubfolder(const QJsonArray &folders, const QString &path)
{
    for (const QJsonValue &value : folders) {
        QJsonObject folder = value.toObject();
        if (folder["path"].toString() == path) {
            return folder["subfolders"].toArray();
        }
        if (folder.contains("subfolders")) {
            QJsonArray subResult = findSubfolder(folder["subfolders"].toArray(), path);
            if (!subResult.isEmpty()) {
                return subResult;
            }
        }
    }
    return QJsonArray();
}

// 点击Slice发生文件改变，发送folderChanged信号
void ChartHandler::handleSliceClicked(const QString &path)
{
    QFileInfo fileInfo(path);
    if(fileInfo.isDir()){
        updatePieChart(path);
        emit folderChanged(path);
    }
}

// 点击回退按钮，发生文件改变，发送folderChanged信号
void ChartHandler::goToParentFolder()
{
    QDir currentDir(curPath);
    if (currentDir.cdUp()) {
        QString parentPath = currentDir.path();
        QFileInfo parentInfo(parentPath);

        if (parentInfo.exists() && parentInfo.isDir()) {
            updatePieChart(parentPath);
            emit folderChanged(parentPath);
        } else {
            // 父目录不存在，显示提示信息
            QMessageBox::warning(nullptr, "警告", "无上级目录或上级目录不存在。");
        }
    } else {
        // 已经在根目录，显示提示信息
        QMessageBox::information(nullptr, "提示", "已经是最顶层目录，无上级目录。");
    }
}


QJsonArray ChartHandler::getSubItemsForPath(const QJsonObject& root, const QString& path)
{
    // 如果当前对象的路径与目标路径匹配，返回其 subItems
    if (root["path"].toString() == path) {
        return root["subItems"].toArray();
    }

    // 如果当前对象有 subItems，递归搜索
    if (root.contains("subItems")) {
        QJsonArray subItems = root["subItems"].toArray();
        for (const QJsonValue& value : subItems) {
            QJsonObject item = value.toObject();

            // 如果找到匹配的路径，返回其 subItems
            if (item["path"].toString() == path) {
                return item["subItems"].toArray();
            }

            // 如果是文件夹（包含 subItems），递归搜索
            if (item.contains("subItems")) {
                QJsonArray result = getSubItemsForPath(item, path);
                if (!result.isEmpty()) {
                    return result;
                }
            }
        }
    }

    // 如果没有找到匹配的路径，返回空数组
    return QJsonArray();
}
