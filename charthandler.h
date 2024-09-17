#ifndef CHARTHANDLER_H
#define CHARTHANDLER_H

#include <QObject>
#include <QJsonObject>
#include <QtCharts/QPieSeries>
#include <QChartView>

class ChartHandler : public QObject
{
    Q_OBJECT
public:
    explicit ChartHandler(QObject *parent = nullptr);
    void updatePieChart(const QJsonObject &results);
    ~ChartHandler();

    QChartView* getChartView() const;


private:
    QChartView* chartView;

    void setupSliceHoverEffects(QPieSlice* slice, const QString& itemName, qint64 size);
    QString formatSize(qint64 bytes);
    QColor generateColorFromIndex(int index);
    // 定义一组颜色
    QList<QColor> colors = {
        QColor(255, 99, 71),   // Tomato
        QColor(30, 144, 255),  // Dodger Blue
        QColor(50, 205, 50),   // Lime Green
        QColor(255, 215, 0),   // Gold
        QColor(138, 43, 226),  // Blue Violet
        QColor(255, 127, 80),  // Coral
        QColor(64, 224, 208),  // Turquoise
        QColor(218, 112, 214), // Orchid
        QColor(255, 192, 203), // Pink
        QColor(100, 149, 237)  // Cornflower Blue
    };
};

#endif // CHARTHANDLER_H
