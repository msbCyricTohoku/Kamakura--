#include "kamakura.h"

#include <QApplication>
//Kamakura-- Mehrdad S. Beni and Hiroshi Watabe, Japan 2023
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    kamakura w;
    w.setWindowIcon(QIcon("resources/icon.ico"));
    w.show();
    return a.exec();
}
//Kamakura-- Mehrdad S. Beni and Hiroshi Watabe, Japan 2023
