#ifndef FILEDETAILSWIDGET_H
#define FILEDETAILSWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QTableWidget>

namespace Ui {
class FileDetailsWidget;
}

class FileDetailsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FileDetailsWidget(QWidget *parent = nullptr);
    ~FileDetailsWidget();

    QString getSelectedFilePath() const;

public slots:
    void updateDetails(const QJsonArray& subItems);

signals:
    void customContextMenuRequested(const QPoint &pos);

protected:
    void contextMenuEvent(QContextMenuEvent *event) override;

private:
    Ui::FileDetailsWidget *ui;

    QLabel *titleLabel;
    QTableWidget *detailsTable;

    void setupUI();
    void loadStyleSheet();
    QString formatSize(qint64 size) const;

    int pathColumn; // 存储文件路径所在的列索引
};

#endif // FILEDETAILSWIDGET_H
