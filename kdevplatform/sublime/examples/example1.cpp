/*
    SPDX-FileCopyrightText: 2006-2007 Alexander Dymo <adymo@kdevelop.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <QApplication>

#include "example1main.h"

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    auto *m = new Example1Main();
    m->show();

    return app.exec();
}

