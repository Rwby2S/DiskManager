#include "filescanner.h"
#include <QDir>
#include <QDirIterator>
#include <QStandardPaths>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFile>
#include <QFileInfo>
#include <QElapsedTimer>
#include <QDebug>

// 构造函数，初始化成员变量
FileScanner::FileScanner(QObject *parent) : QObject(parent), totalFiles(0), scannedFiles(0) {}

// 开始扫描函数
void FileScanner::startScan(const QString &path)
{
    totalFiles = 0; // 初始化总文件数
    scannedFiles = 0; // 初始化已扫描文件数
    int lastReportedPercentage = -1;

    // 使用一个迭代器来遍历所有子目录和文件，同时统计总文件数
    QDirIterator countIt(path, QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    while (countIt.hasNext()) {
        countIt.next();
        totalFiles++;
    }

    qDebug() << "Total files to scan:" << totalFiles;

    // 执行扫描并获取结果
    ScanResult result = scanDirectory(path, lastReportedPercentage);
    // 将结果转换为JSON对象
    QJsonObject jsonResult = convertToJson(result);
    // 将结果保存为JSON文件
    saveToJson(path, jsonResult);
    // 发射扫描完成信号
    emit scanFinished(jsonResult);
}

// 递归扫描目录的函数
ScanResult FileScanner::scanDirectory(const QString &path, int lastReportedPercentage)
{
    ScanResult result;
    QDir dir(path);
    QElapsedTimer timer;
    timer.start();  // 开始计时

    // 使用QDirIterator递归遍历目录中的所有文件和子目录
    QDirIterator it(path, QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
    while (it.hasNext()) {
        QString filePath = it.next();
        QFileInfo fileInfo(filePath);
        QString absolutePath = fileInfo.absoluteFilePath();

        if (fileInfo.isDir()) {
            // 处理子目录，递归调用scanDirectory
            ScanResult subDirResult = scanDirectory(absolutePath, lastReportedPercentage);
            result.subfolders[absolutePath] = subDirResult;
            result.folderSizes[absolutePath] = subDirResult.folderSizes[absolutePath];
            result.folderSizes[path] += result.folderSizes[absolutePath];
        } else {
            // 计算文件大小并累加到相应的文件夹中
            qint64 size = fileInfo.size();

            // 更新文件所在的所有父目录的大小
            QString currentPath = absolutePath;
            while (currentPath != path && currentPath != "") {
                result.folderSizes[currentPath] += size;
                currentPath = QFileInfo(currentPath).path();
            }
            result.folderSizes[path] += size; // 更新根目录大小

            scannedFiles++;
        }

        // 计算并发射进度更新信号
        int percentage = (scannedFiles * 100) / totalFiles;
        if (scannedFiles % 10 == 0 || percentage > lastReportedPercentage) {  // 每扫描10个文件或百分比增加时更新
            emit progressUpdated(percentage);
            lastReportedPercentage = percentage;

            // 计算扫描速度
            double elapsedSeconds = timer.elapsed() / 1000.0;
            double scanSpeed = scannedFiles / elapsedSeconds;

            qDebug() << "Scanned" << scannedFiles << "of" << totalFiles << "files. Current size:"
                     << result.folderSizes[path] << "B. Scan speed:" << qRound(scanSpeed) << "files/s";
        }
    }

    return result;
}

// 将扫描结果转换为JSON格式
QJsonObject FileScanner::convertToJson(const ScanResult &result)
{
    QJsonObject json;
    QJsonArray folders;

    // 遍历所有文件夹大小并将其转换为JSON对象
    for (auto it = result.folderSizes.begin(); it != result.folderSizes.end(); ++it) {
        QJsonObject folder;
        folder["path"] = it.key();
        folder["size"] = it.value();

        if (result.subfolders.contains(it.key())) {
            folder["subfolders"] = convertToJson(result.subfolders[it.key()]);
        }

        folders.append(folder);
    }

    json["folders"] = folders;
    return json;
}

// 将扫描结果保存为JSON文件
void FileScanner::saveToJson(const QString &path, const QJsonObject &result)
{
    // 获取用户的文档目录
    QString jsonDirPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/ScanResults";

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
        QJsonDocument document(result);
        file.write(document.toJson(QJsonDocument::Indented));
        file.close();
        qDebug() << "Scan result saved to" << jsonFilePath;
    } else {
        qWarning() << "Failed to save scan result to" << jsonFilePath;
    }
}

// 将字节数转换为合适的单位（KB、MB、GB、TB）
//QString FileScanner::formatSize(qint64 bytes)
//{
//    const qint64 KILOBYTE = 1024;
//    const qint64 MEGABYTE = KILOBYTE * 1024;
//    const qint64 GIGABYTE = MEGABYTE * 1024;
//    const qint64 TERABYTE = GIGABYTE * 1024;

//    if (bytes >= TERABYTE)
//        return QString::number(bytes / (double)TERABYTE, 'f', 2) + " TB";
//    else if (bytes >= GIGABYTE)
//        return QString::number(bytes / (double)GIGABYTE, 'f', 2) + " GB";
//    else if (bytes >= MEGABYTE)
//        return QString::number(bytes / (double)MEGABYTE, 'f', 2) + " MB";
//    else if (bytes >= KILOBYTE)
//        return QString::number(bytes / (double)KILOBYTE, 'f', 2) + " KB";
//    else
//        return QString::number(bytes) + " B";
//}
