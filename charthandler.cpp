#include "charthandler.h"
#include <QtCharts/QChart>
#include <QtCharts/QPieSeries>
#include <QJsonArray>
#include <QJsonObject>
#include <QFileInfo>
#include <QDebug>

ChartHandler::ChartHandler(QObject *parent)
    : QObject{parent}
{
    chartView = new QChartView(); // 初始化图表视图
    chartView->setRenderHint(QPainter::Antialiasing);
}

ChartHandler::~ChartHandler()
{
    delete chartView;
}

// 获取图表视图
QChartView* ChartHandler::getChartView() const
{
    return chartView;
}


void ChartHandler::updatePieChart(const QJsonObject &results)
{
    QPieSeries *series = new QPieSeries();
    int i = 0;
    QString title = "Disk Usage by Folder: ";

    // 遍历扫描结果，生成饼状图数据
    QJsonArray folders = results["folders"].toArray();
    for (const QJsonValue &value : folders) {
        QJsonObject folder = value.toObject();
        QString folderPath = folder["path"].toString();
        double size = folder["size"].toDouble();

        QFileInfo fileInfo(folderPath);
        QString folderName = fileInfo.fileName();

        if (i++ == 0){
            title += folderName;
            continue;
        }
        // 处理文件夹名称过长的情况
        if (folderName.length() > 15) {
            folderName = folderName.left(12) + "...";
        }

        QPieSlice *slice = series->append(folderName, size);
        slice->setLabelVisible(false);  // 默认隐藏标签

        // 设置鼠标悬浮效果
        setupSliceHoverEffects(slice, folderName, size);
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
