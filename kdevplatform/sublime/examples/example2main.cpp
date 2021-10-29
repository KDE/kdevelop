/*
    SPDX-FileCopyrightText: 2007 Alexander Dymo <adymo@kdevelop.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "example2main.h"

#include <QMenu>

#include <QMenuBar>

#include <sublime/area.h>
#include <sublime/view.h>
#include <sublime/document.h>

Example2Main::Example2Main(Sublime::Controller *controller, Qt::WindowFlags flags)
    :Sublime::MainWindow(controller, flags)
{
    auto* windowMenu = new QMenu(QStringLiteral("Window"), this);
    windowMenu->addAction(QStringLiteral("Split Top/Bottom"), this, SLOT(splitVertical()), Qt::CTRL | Qt::Key_T);
    windowMenu->addAction(QStringLiteral("Split Left/Right"), this, SLOT(splitHorizontal()), Qt::CTRL | Qt::Key_L);
    windowMenu->addSeparator();
    windowMenu->addAction(QStringLiteral("Close"), this, SLOT(close()), Qt::CTRL | Qt::Key_W);
    menuBar()->addMenu(windowMenu);
}

void Example2Main::splitVertical()
{
    if (!activeView())
        return;
    Sublime::View *newView = activeView()->document()->createView();
    area()->addView(newView, activeView(), Qt::Vertical);
    activateView(newView);
}

void Example2Main::splitHorizontal()
{
    if (!activeView())
        return;
    Sublime::View *newView = activeView()->document()->createView();
    area()->addView(newView, activeView(), Qt::Horizontal);
    activateView(newView);
}

void Example2Main::close()
{
    if (!activeView() || area()->views().count() == 1)
        return;
    delete area()->removeView(activeView());
}


