#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("Typing Speed Test");
    app.setOrganizationName("CS Department");

    MainWindow window;
    window.show();
    return app.exec();
}
