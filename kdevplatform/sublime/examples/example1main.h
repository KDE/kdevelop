/*
    SPDX-FileCopyrightText: 2006-2007 Alexander Dymo <adymo@kdevelop.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_EXAMPLE1MAIN_H
#define KDEVPLATFORM_EXAMPLE1MAIN_H

#include <KXmlGuiWindow>

namespace Sublime {
    class Area;
    class Controller;
}

class Example1Main: public KXmlGuiWindow {
    Q_OBJECT
public:
    Example1Main();

public Q_SLOTS:
    void selectArea1();
    void selectArea2();

private Q_SLOTS:
    void updateTitle(Sublime::Area *area);

private:
    Sublime::Controller *m_controller;

    Sublime::Area *m_area1;
    Sublime::Area *m_area2;

};

#endif

