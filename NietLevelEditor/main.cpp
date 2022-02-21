#include "MainWindow.hpp"
#include "GridEditor.hpp"
#include <QApplication>
#include <QLocale>

//======================================================================
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
