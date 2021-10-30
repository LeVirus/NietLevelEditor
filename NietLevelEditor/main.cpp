#include "MainWindow.hpp"
#include "GridEditor.hpp"
#include <QApplication>
#include <QLocale>

//======================================================================
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    //TEST
    GridEditor grid;
    grid.initGrid("/home/cyril/Programmation/Nietsneflow/Nietsneflow3d", 150, 300);
    return grid.exec();
    //TEST
    MainWindow w;
    w.show();
    return a.exec();
}
