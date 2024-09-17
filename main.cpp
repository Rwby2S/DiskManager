#include "fileanalyzer.h"

#include <QApplication>
#include <QStyleFactory>

#include <QFile>
#include <QTextStream>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Set application palette
    QPalette palette;
    palette.setColor(QPalette::Window, QColor(240, 240, 240));
    a.setPalette(palette);

    // Load and apply the style sheet
    QFile styleFile(":/resource/styles.qss");
    if (styleFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream styleStream(&styleFile);
        a.setStyleSheet(styleStream.readAll());
        styleFile.close();
    }
    else
    {
        qWarning() << "Unable to open style sheet file";
    }

    FileAnalyzer w;
    w.show();
    return a.exec();
}
