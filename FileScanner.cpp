#include "filescanner.h"
#include <QDir>
#include <QDirIterator>

#include <QJsonArray>
#include <QJsonDocument>
#include <QFile>
#include <QFileInfo>
#include <QElapsedTimer>
#include <QDebug>

// 构造函数，初始化成员变量
FileScanner::FileScanner(QObject *parent) : QObject(parent), isCancelled(false)
{
    fileWatcher = new QFileSystemWatcher(this);

    rescanTimer = new QTimer(this);
    rescanTimer->setSingleShot(true);
    rescanTimer->setInterval(5000);

    connect(fileWatcher, &QFileSystemWatcher::fileChanged, this, &FileScanner::onFileChanged);
    connect(fileWatcher, &QFileSystemWatcher::directoryChanged, this, &FileScanner::onDirectoryChanged);
    connect(rescanTimer, &QTimer::timeout, this, [this]() { updateIncrementally(rootPath); });

    loadCachedResults();
}

FileScanner::~FileScanner()
{
    saveCachedResults();
    delete fileWatcher;
    delete rescanTimer;
}

// 开始扫描函数
void FileScanner::startScan(const QString &path)
{
    rootPath = path;
    isCancelled = false;

    QThread *thread = QThread::create([this, path]{
        QJsonObject results;
        scanDirectory(path, results);
        if(!isCancelled){
            cachedResults = results;
            saveCachedResults();
            emit scanFinished();
        }
    });
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);
    thread->start();
}

void FileScanner::cancleScan()
{
    isCancelled = true;
}

// 递归扫描目录的函数
void FileScanner::scanDirectory(const QString& path, QJsonObject& results) {
    QDirIterator it(path, QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    qint64 totalSize = 0;
    int fileCount = 0;
    QJsonArray subItems;

    while (it.hasNext() && !isCancelled) {
        QString filePath = it.next();
        QFileInfo fileInfo(filePath);

        QJsonObject item;
        item["path"] = filePath;

        if (fileInfo.isDir()) {
            QJsonObject subFolder;
            scanDirectory(filePath, subFolder);
            item["size"] = subFolder["size"];
            if (subFolder.contains("subItems")) {
                item["subItems"] = subFolder["subItems"];
            }
            totalSize += subFolder["size"].toString().toLongLong();
        } else {
            item["size"] = QString::number(fileInfo.size());
            totalSize += fileInfo.size();
        }

        subItems.append(item);
        fileWatcher->addPath(filePath);

        if (++fileCount % 100 == 0) {
            emit progressUpdated(fileCount);
        }
    }

    results["path"] = path;
    results["size"] = QString::number(totalSize);
    if (!subItems.isEmpty()) {
        results["subItems"] = subItems;
    }
}

void FileScanner::loadCachedResults()
{
    QString jsonFilePath = jsonDirPath + "/scan_result.json";
    QFile file(jsonFilePath);
    if (file.open(QIODevice::ReadOnly)) {
        cachedResults = QJsonDocument::fromJson(file.readAll()).object();
        file.close();
    }
}

// 将扫描结果保存为JSON文件
void FileScanner::saveCachedResults()
{
    // 获取用户的文档目录
    // 确保目录存在，如果不存在则创建
    if (QDir().mkpath(jsonDirPath)) {
        qDebug() << "Directory created or already exists:" << jsonDirPath;
    } else {
        qWarning() << "Failed to create directory:" << jsonDirPath;
        return; // 如果创建目录失败，则返回
    }

    // 构建完整的JSON文件路径
    QString jsonFilePath = jsonDirPath + "/scan_result.json";
    QFile file(jsonFilePath);

    if (file.open(QIODevice::WriteOnly)) {
        QJsonDocument document(cachedResults);
        file.write(document.toJson(QJsonDocument::Indented));
        file.close();
        qDebug() << "Scan result saved to" << jsonFilePath;
    } else {
        qWarning() << "Failed to save scan result to" << jsonFilePath;
    }
}

void FileScanner::onFileChanged(const QString &path)
{
    rescanTimer->start();
}

void FileScanner::onDirectoryChanged(const QString &path)
{
    rescanTimer->start();
}

void FileScanner::updateIncrementally(const QString &path)
{
    QJsonObject newResults;
    scanDirectory(path, newResults);
    cachedResults = newResults;
    saveCachedResults();
    emit scanFinished();
}
