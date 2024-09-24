#include "contextmenuhandler.h"

ContextMenuHandler::ContextMenuHandler(QObject *parent)
    : QObject{parent}
{
    contextMenu = new QMenu(static_cast<QWidget*>(parent));

    openInExplorerAction = new QAction("Open in Explorer", this);
    contextMenu->addAction(openInExplorerAction);

    // 将QAction的triggered信号与handler的openExplorer信号连接
    // ContextMenuHandler 重新发射信号，让FileAnalyzer可以连接到该信号，并执行相应的槽函数
    connect(openInExplorerAction, &QAction::triggered, this, &ContextMenuHandler::openInExplorerRequested);
}

ContextMenuHandler::~ContextMenuHandler()
{
    delete contextMenu;
    delete openInExplorerAction;
}

void ContextMenuHandler::showContextMenu(const QPoint& pos)
{
    contextMenu->exec(pos);
}
