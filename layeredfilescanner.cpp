#include "layeredFileScanner.h"
#include <QDirIterator>
#include <QDebug>
#include <QtConcurrent/QtConcurrent>


LayeredFileScanner::LayeredFileScanner(QObject *parent) : FileScanner(parent)
{
    threadPool.setMaxThreadCount(QThread::idealThreadCount());
}

LayeredFileScanner::~LayeredFileScanner() = default;

void LayeredFileScanner::startScan(const QString& path) {
    rootPath = path;
    isCancelled = false;

    QElapsedTimer timer;
    timer.start();

    loadCachedResults();

    if (cachedResults["path"].toString() == path) {
        // 增量更新
        cachedResults = updateNode(cachedResults);
    } else {
        // 完全重新扫描
        cachedResults = scanDirectory(path);
    }

    saveCachedResults();

    qDebug() << "Scan completed in" << timer.elapsed() << "ms";
    emit scanFinished();
}

QJsonObject LayeredFileScanner::scanDirectory(const QString& path) {
    QJsonObject result;
    result["path"] = path;

    QJsonArray subItems;
    qint64 totalSize = 0;

    QDir dir(path);
    QFileInfoList entries = dir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Hidden);

    QList<QFuture<QJsonObject>> futures;

    for (const QFileInfo &info : entries) {
        if (isCancelled) break;

        QJsonObject item;

        if (info.isFile()) {
            QJsonObject item;
            item["path"] = info.filePath();
            qint64 fileSize = info.size();
            item["size"] = QString::number(fileSize);
            item["hash"] = calculateFileHash(info.filePath());
            totalSize += fileSize;
            subItems.append(item);
        } else if (info.isDir()) {
            // 第一次存储，遍历目录的所有目录，添加异步任务
            futures.append(QtConcurrent::run(&threadPool, [this, &info]() {
                return scanDirectory(info.filePath());
            }));
        }
    }
    // 处理异步任务的结果时，再次将子目录的完整扫描结果添加到subItems中
    for (QFuture<QJsonObject>& future : futures) {
        if (isCancelled) break;
        QJsonObject subDir = future.result();
        subItems.append(subDir);
        totalSize += subDir["size"].toString().toLongLong();
    }

    result["size"] = QString::number(totalSize);
    result["subItems"] = subItems;

    return result;
}

QJsonObject LayeredFileScanner::updateNode(const QJsonObject& node) {
    QJsonObject updatedNode = node;
    QFileInfo fileInfo(node["path"].toString());

    if (!fileInfo.exists()) {
        // 文件或文件夹已被删除
        return QJsonObject();
    }

    if (fileInfo.isFile()) {
        qint64 newSize = fileInfo.size();
        QString newHash = calculateFileHash(fileInfo.filePath());
        if (newSize != node["size"].toString().toLongLong() || newHash != node["hash"].toString()) {
            updatedNode["size"] = QString::number(newSize);
            updatedNode["hash"] = newHash;
        }
    } else if (fileInfo.isDir()) {
        QJsonArray updatedSubItems;
        qint64 totalSize = 0;

        QDir dir(fileInfo.filePath());
        QFileInfoList currentEntries = dir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Hidden);
        QJsonArray oldSubItems = node["subItems"].toArray();

        QList<QFuture<QJsonObject>> futures;

        for (const QFileInfo &info : currentEntries) {
            if (isCancelled) break;

            auto it = std::find_if(oldSubItems.begin(), oldSubItems.end(), [&info](const QJsonValue &value) {
                return value.toObject()["path"].toString() == info.filePath();
            });

            if (it != oldSubItems.end()) {
                // 更新现有项
                futures.append(QtConcurrent::run(&threadPool, [this, item = it->toObject()]() {
                    return updateNode(item);
                }));
            } else {
                // 添加新项
                futures.append(QtConcurrent::run(&threadPool, [this, &info]() {
                    return scanDirectory(info.filePath());
                }));
            }
        }

        for (QFuture<QJsonObject>& future : futures) {
            if (isCancelled) break;
            QJsonObject subItem = future.result();
            if (!subItem.isEmpty()) {
                updatedSubItems.append(subItem);
                totalSize += subItem["size"].toString().toLongLong();
            }
        }

        updatedNode["subItems"] = updatedSubItems;
        updatedNode["size"] = QString::number(totalSize);
    }

    return updatedNode;
}

QString LayeredFileScanner::calculateFileHash(const QString& filePath) {
    QFile file(filePath);
    if (file.open(QFile::ReadOnly)) {
        QCryptographicHash hash(QCryptographicHash::Md5);
        if (hash.addData(&file)) {
            return hash.result().toHex();
        }
    }
    return QString();
}

