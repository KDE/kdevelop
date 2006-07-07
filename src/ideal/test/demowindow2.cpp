/***************************************************************************
 *   Copyright (C) 2006 by Alexander Dymo                                  *
 *   adymo@kdevelop.org                                                    *
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
#include "demowindow2.h"

#include <QAction>
#include <QMenu>
#include <QMenuBar>
#include <QTextEdit>

#include <kdebug.h>

#include "area.h"

DemoWindow2::DemoWindow2(QWidget *parent)
    :MainWindow(parent)
{
    setCentralWidget(new QTextEdit());
    createActions();
    createToolViews();
    setArea(Area::Code);
    resize(800, 600);
}

void DemoWindow2::createToolViews()
{
    QWidget *tv = new QTextEdit(0); tv->setWindowTitle("Breakpoints");
    addToolView(tv, Ideal::Left, Area::Debug);

    tv = new QTextEdit(0); tv->setWindowTitle("File Selector");
    addToolView(tv, Ideal::Left, Area::Code | Area::Debug);

    tv = new QTextEdit(0); tv->setWindowTitle("Messages");
    addToolView(tv, Ideal::Bottom, Area::Code | Area::Debug);

    tv = new QTextEdit(0); tv->setWindowTitle("Konsole");
    addToolView(tv, Ideal::Bottom, Area::Code | Area::Debug);

    tv = new QTextEdit(0); tv->setWindowTitle("GDB");
    addToolView(tv, Ideal::Bottom, Area::Debug);

    tv = new QTextEdit(0); tv->setWindowTitle("Documentation");
    addToolView(tv, Ideal::Right, Area::Code | Area::Design);
}

void DemoWindow2::createActions()
{
    areaMenu = menuBar()->addMenu("Area");

    QActionGroup *areaGroup = new QActionGroup(this);
    areaGroup->setExclusive(true);

    defaultArea = new QAction("Default", this);
    connect(defaultArea, SIGNAL(triggered()), this, SLOT(selectArea()));
    defaultArea->setCheckable(true);
    areaGroup->addAction(defaultArea);
    areaMenu->addAction(defaultArea);

    codeArea = new QAction("Code", this);
    connect(codeArea, SIGNAL(triggered()), this, SLOT(selectArea()));
    codeArea->setCheckable(true);
    codeArea->setChecked(true);
    areaGroup->addAction(codeArea);
    areaMenu->addAction(codeArea);

    debugArea = new QAction("Debug", this);
    connect(debugArea, SIGNAL(triggered()), this, SLOT(selectArea()));
    debugArea->setCheckable(true);
    areaGroup->addAction(debugArea);
    areaMenu->addAction(debugArea);

    designArea = new QAction("Design", this);
    connect(designArea, SIGNAL(triggered()), this, SLOT(selectArea()));
    designArea->setCheckable(true);
    areaGroup->addAction(designArea);
    areaMenu->addAction(designArea);
}

void DemoWindow2::selectArea()
{
    QAction *action = qobject_cast<QAction*>(sender());
    if (action == defaultArea)
        setArea(Area::Default);
    else if (action == codeArea)
        setArea(Area::Code);
    else if (action == debugArea)
        setArea(Area::Debug);
    else if (action == designArea)
        setArea(Area::Design);
}

#include "demowindow2.moc"
