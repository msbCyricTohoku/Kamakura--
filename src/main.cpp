#include "kamakura.h"

#include <QApplication>
//Kamakura-- Mehrdad S. Beni and Hiroshi Watabe, Japan 2023
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    kamakura w;
    // Use the resource system so the application icon is bundled in the binary
    // The path corresponds to the prefix defined in resources.qrc
    w.setWindowIcon(QIcon(":/new/prefix1/resources/icon.ico"));
    w.show();
    return a.exec();
}
//Kamakura-- Mehrdad S. Beni and Hiroshi Watabe, Japan 2023
