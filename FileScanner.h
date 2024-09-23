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

    virtual void startScan(const QString& path) = 0;
    void cancleScan();

    const QJsonObject& getCachedResults() const { return cachedResults; }

signals:
    void progressUpdated(int percentage);
    void scanFinished();

private slots:
    void onFileChanged(const QString& path);
    void onDirectoryChanged(const QString& results);

protected:
    QString rootPath;
    QJsonObject cachedResults;
    QFileSystemWatcher *fileWatcher;
    QTimer *rescanTimer;
    bool isCancelled;

    virtual void scanDirectory(const QString& path, QJsonObject& results);
    QJsonObject doScanDirectory(const QString& path);
    void loadCachedResults();
    void saveCachedResults();
    void updateIncrementally(const QString &path);

private:
    QThread scanThread;
    QString jsonDirPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/ScanResults";

    void saveToJson(const QString& path, const QJsonObject& result);


};

#endif // FILESCANNER_H
