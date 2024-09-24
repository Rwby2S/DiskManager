#ifndef CONTEXTMENUHANDLER_H
#define CONTEXTMENUHANDLER_H

#include <QObject>
#include <QMenu>
#include <QAction>

class ContextMenuHandler : public QObject
{
    Q_OBJECT
public:
    explicit ContextMenuHandler(QObject *parent = nullptr);

    ~ContextMenuHandler();

    void showContextMenu(const QPoint& pos);

signals:
    void openInExplorerRequested();

private:
    QMenu *contextMenu;

    QAction *openInExplorerAction;
};


#endif // CONTEXTMENUHANDLER_H
