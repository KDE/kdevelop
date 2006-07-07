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

#ifndef DEMOWINDOW2_H
#define DEMOWINDOW2_H

#include <mainwindow.h>

class QMenu;
class QAction;

using namespace Ideal;

class DemoWindow2: public MainWindow {
    Q_OBJECT
public:
    DemoWindow2(QWidget *parent = 0);

private slots:
    void selectArea();

private:
    void createToolViews();
    void createActions();

    QMenu *areaMenu;
    QAction *defaultArea;
    QAction *codeArea;
    QAction *debugArea;
    QAction *designArea;

};

#endif
