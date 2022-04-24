#include "mainwindow.h"
#include <QApplication>

int currentExitCode = 0;

int main(int argc, char *argv[])
{
    do {
        QApplication a(argc, argv);
        MainWindow main_window;
        main_window.show();
        currentExitCode = a.exec();
    } while (currentExitCode == EXIT_CODE_REBOOT);

    return currentExitCode;
}
