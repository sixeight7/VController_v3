#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Create and start main window
    MainWindow main_window;
    main_window.show();

    return a.exec();
}
