/*
    SPDX-FileCopyrightText: 2011 Julien Desgats <julien.desgats@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

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
