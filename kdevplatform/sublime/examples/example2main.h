/*
    SPDX-FileCopyrightText: 2007 Alexander Dymo <adymo@kdevelop.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_EXAMPLE2MAIN_H
#define KDEVPLATFORM_EXAMPLE2MAIN_H

#include <sublime/mainwindow.h>

class Example2Main: public Sublime::MainWindow {
    Q_OBJECT

public:
    explicit Example2Main(Sublime::Controller *controller, Qt::WindowFlags flags = {});

public Q_SLOTS:
    void splitVertical();
    void splitHorizontal();
    void close();

};

#endif

