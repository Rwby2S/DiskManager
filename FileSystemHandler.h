#ifndef FILESYSTEMHANDLER_H
#define FILESYSTEMHANDLER_H

#include <QObject>
#include <QString>
#include <QFileSystemModel>


class FileSystemHandler : public QObject
{
    Q_OBJECT
public:
    explicit FileSystemHandler(QObject* parent = nullptr);
    ~FileSystemHandler();

    QFileSystemModel* getFileSystemModel();
    QString getSelectedPath(const QModelIndex& index);

private:
    QFileSystemModel* fileSystemModel;
};

#endif // FILESYSTEMHANDLER_H
