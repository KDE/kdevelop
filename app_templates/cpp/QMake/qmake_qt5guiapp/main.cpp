#include <QApplication>
#include <QMainWindow>

#include "ui_mainwindow.h"

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    QMainWindow w;
    Ui::MainWindow main;
    main.setupUi(&w);
    w.show();

    return app.exec();
}
