#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    //Parse args for hte serial number of camera.
    QApplication a(argc, argv);

    MainWindow* w = new MainWindow();
    w->show();
    w->resize(500,500);

    return a.exec();
}
