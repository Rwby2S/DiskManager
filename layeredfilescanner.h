#ifndef LAYEREDFILESCANNER_H
#define LAYEREDFILESCANNER_H

#include "FileScanner.h"
#include <QThreadPool>
#include <QFuture>
#include <QAtomicInteger>
#include <QElapsedTimer>

class LayeredFileScanner : public FileScanner
{
    Q_OBJECT

public:
    explicit LayeredFileScanner(QObject *parent = nullptr);
    ~LayeredFileScanner() override;

    void startScan(const QString& path) override;
    qint64 getFolderSize(const QString &folderPath);

protected:
//    QJsonObject LayeredFileScanner::scanDirectory(const QString& path);
    // 覆盖基类的 scanDirectory 方法
    void scanDirectory(const QString& path, QJsonObject& results) override;

private:
    QThreadPool threadPool;
    QAtomicInteger<qint64> totalScanned;
    qint64 totalToScan;
    QElapsedTimer scanTimer;

    void estimateTotalSize(const QString &path);
    QJsonObject scanRecursive(const QString &path, int depth, int maxDepth = 3);

    QJsonObject scanDirectory(const QString& path);
    QJsonObject updateNode(const QJsonObject& node);
    QString calculateFileHash(const QString& filePath);
    void processNode(QJsonObject& node);
};

#endif // LAYEREDFILESCANNER_H
