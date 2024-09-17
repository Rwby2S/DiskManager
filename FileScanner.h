#ifndef FILESCANNER_H
#define FILESCANNER_H

#include <QObject>
#include <QString>
#include <QMap>
#include <QJsonObject>

struct ScanResult
{
    QMap<QString, qint64> folderSizes;
    QMap<QString, ScanResult> subfolders;
};

class FileScanner : public QObject
{
    Q_OBJECT

public:
    explicit FileScanner(QObject *parent = nullptr);
    void startScan(const QString& path);

signals:
    void progressUpdated(int percentage);
    void scanFinished(const QJsonObject &results);

private:
    ScanResult scanDirectory(const QString &path, int lastReportedPercentage);
    QJsonObject convertToJson(const ScanResult& result);
    void saveToJson(const QString& path, const QJsonObject& result);
//    QString formatSize(qint64 bytes);

    int totalFiles;
    int scannedFiles;
};

#endif // FILESCANNER_H
