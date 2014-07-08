/***************************************************************************
 *   Copyright 2007 Alexander Dymo <adymo@kdevelop.org>                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/
#include "example2main.h"

#include <QMenu>

#include <kdebug.h>
#include <kmenubar.h>

#include <sublime/area.h>
#include <sublime/view.h>
#include <sublime/document.h>

Example2Main::Example2Main(Sublime::Controller *controller, Qt::WindowFlags flags)
    :Sublime::MainWindow(controller, flags)
{
    QMenu *windowMenu = new QMenu("Window", this);
    windowMenu->addAction("Split Top/Bottom", this, SLOT(splitVertical()), Qt::CTRL + Qt::Key_T);
    windowMenu->addAction("Split Left/Right", this, SLOT(splitHorizontal()), Qt::CTRL + Qt::Key_L);
    windowMenu->addSeparator();
    windowMenu->addAction("Close", this, SLOT(close()), Qt::CTRL + Qt::Key_W);
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


