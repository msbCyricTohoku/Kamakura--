#include "kamakura.h"

#include <QApplication>
#include <QPalette>
#include <QColor>
//Kamakura-- Mehrdad S. Beni and Hiroshi Watabe, Japan 2023
static void applyDarkTheme(QApplication& app)
{
    app.setStyle("Fusion");
    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, QColor(53,53,53));
    darkPalette.setColor(QPalette::WindowText, Qt::white);
    darkPalette.setColor(QPalette::Base, QColor(42,42,42));
    darkPalette.setColor(QPalette::AlternateBase, QColor(66,66,66));
    darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
    darkPalette.setColor(QPalette::ToolTipText, Qt::white);
    darkPalette.setColor(QPalette::Text, Qt::white);
    darkPalette.setColor(QPalette::Button, QColor(53,53,53));
    darkPalette.setColor(QPalette::ButtonText, Qt::white);
    darkPalette.setColor(QPalette::BrightText, Qt::red);
    darkPalette.setColor(QPalette::Highlight, QColor(142,45,197).lighter());
    darkPalette.setColor(QPalette::HighlightedText, Qt::black);
    app.setPalette(darkPalette);
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    applyDarkTheme(a);
    kamakura w;
    // Use the resource system so the application icon is bundled in the binary
    // The path corresponds to the prefix defined in resources.qrc
    w.setWindowIcon(QIcon(":/new/prefix1/resources/icon.ico"));
    w.show();
    return a.exec();
}
//Kamakura-- Mehrdad S. Beni and Hiroshi Watabe, Japan 2023
