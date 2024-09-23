#ifndef CHARTHANDLER_H
#define CHARTHANDLER_H

#include "FileScanner.h"
#include <QObject>
#include <QJsonObject>
#include <QtCharts/QPieSeries>
#include <QChartView>
#include <QPushButton>

class ChartHandler : public QObject
{
    Q_OBJECT
public:
    explicit ChartHandler(QObject *parent = nullptr, FileScanner* scanner = nullptr);
    void updatePieChart(const QString &currentPath = QString());
    ~ChartHandler();

    QChartView* getChartView() const;


signals:
    void folderChanged(const QString& path);
    void directoryDataUpdated(const QJsonArray& directoryInfo);

public slots:
    void goToParentFolder();

private:
    QChartView* chartView;
    QString curPath;
    QJsonObject fullResults;
    FileScanner* fileScanner;

    void setupSliceHoverEffects(QPieSlice* slice, const QString& itemName, qint64 size);

    QString formatSize(qint64 bytes);

    QColor generateColorFromIndex(int index);

//    QString createChartTitle(const QString& path);

    void handleSliceClicked(const QString &path);

    QJsonArray findSubfolder(const QJsonArray &folders, const QString &path);

    QJsonArray getSubItemsForPath(const QJsonObject& root, const QString& path);

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
