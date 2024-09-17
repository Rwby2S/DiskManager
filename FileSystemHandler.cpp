#include "fileSystemHandler.h"

// FileSystemHandler 构造函数定义
FileSystemHandler::FileSystemHandler(QObject *parent) : QObject(parent)
{
    // 初始化文件系统模型
    fileSystemModel = new QFileSystemModel(this);

    // 设置文件系统模型的根路径为系统的根目录
    fileSystemModel->setRootPath(""); // 设置为空字符串表示根目录， !如果设置成QDir::rootPath()，则设置成C盘目录

    // 设置文件系统模型的过滤器，仅显示可读的目录和文件
    fileSystemModel->setFilter(QDir::NoDotAndDotDot | QDir::AllDirs | QDir::Files);

    // 设置文件系统模型要显示的列：名称、大小、类型、修改日期
    fileSystemModel->setNameFilters(QStringList() << "*");
    fileSystemModel->setNameFilterDisables(false); // 启用过滤
}

FileSystemHandler::~FileSystemHandler()
{
    // 析构函数中删除文件系统模型
    delete fileSystemModel;
}

// 获取fileSystemModel
QFileSystemModel* FileSystemHandler::getFileSystemModel()
{
    return fileSystemModel;
}

// 获取选中文件或目录的绝对路径
QString FileSystemHandler::getSelectedPath(const QModelIndex& index)
{
    return fileSystemModel->filePath(index);
}
