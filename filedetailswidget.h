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

public slots:
    void updateDetails(const QJsonArray& subItems);

private:
    Ui::FileDetailsWidget *ui;

    QLabel *titleLabel;
    QTableWidget *detailsTable;

    void setupUI();
    void loadStyleSheet();
    QString formatSize(qint64 size) const;
};

#endif // FILEDETAILSWIDGET_H
