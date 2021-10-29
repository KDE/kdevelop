/*
    SPDX-FileCopyrightText: 2006-2007 Alexander Dymo <adymo@kdevelop.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "mainwindowoperator.h"

#include "mainwindow.h"

namespace Sublime {

void MainWindowOperator::setArea(MainWindow *w, Area *area)
{
    w->setArea(area);
}

void MainWindowOperator::clearArea(MainWindow *w)
{
    w->clearArea();
}

void MainWindowOperator::setActiveView(MainWindow *w, View *view)
{
    w->activateView(view);
}

void MainWindowOperator::setActiveToolView(MainWindow *w, View *view)
{
    w->setActiveToolView(view);
}

}
