#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.setWindowTitle("MedDesk - Gestionare pacienti");
    w.show();
    return QApplication::exec();
}
