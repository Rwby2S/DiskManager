#ifndef FILESCANNER_H
#define FILESCANNER_H

#include <QObject>
#include <QString>
#include <QMap>
#include <QThread>
#include <QTimer>
#include <QJsonObject>
#include <QFileSystemWatcher>
#include <QStandardPaths>

//struct ScanResult
//{
//    QMap<QString, qint64> folderSizes;
//    QMap<QString, ScanResult> subfolders;
//};

class FileScanner : public QObject
{
    Q_OBJECT

public:
    explicit FileScanner(QObject *parent = nullptr);
    ~FileScanner();

    void startScan(const QString& path);
    void cancleScan();

    const QJsonObject& getCachedResults() const { return cachedResults; }

signals:
    void progressUpdated(int percentage);
    void scanFinished();

private slots:
    void onFileChanged(const QString& path);
    void onDirectoryChanged(const QString& results);

private:
    QThread scanThread;
    QFileSystemWatcher *fileWatcher;
    QTimer *rescanTimer;
    QJsonObject cachedResults;

    QString rootPath;
    bool isCancelled;
    QString jsonDirPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/ScanResults";


    void scanDirectory(const QString &path, QJsonObject &results);
//    QJsonObject convertToJson(const ScanResult& result);
    void saveToJson(const QString& path, const QJsonObject& result);

    void saveCachedResults();
    void loadCachedResults();
    void updateIncrementally(const QString &path);

//    int totalFiles;
//    int scannedFiles;
};

#endif // FILESCANNER_H
