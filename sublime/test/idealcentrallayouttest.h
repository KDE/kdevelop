/***************************************************************************
 *   Copyright 2006-2007 Alexander Dymo  <adymo@kdevelop.org>       *
 *   Copyright 2008 Hamish Rodda <rodda@kde.org>                           *
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
#ifndef SUBLIMEIDEALCENTRALLAYOUTTEST_H
#define SUBLIMEIDEALCENTRALLAYOUTTEST_H

#include <QObject>

namespace Sublime {
    class Area;
    class View;
    class Controller;
    class MainWindow;
    class IdealCentralWidget;
}

class IdealCentralLayoutTest: public QObject {
    Q_OBJECT
private slots:
    void init();
    void cleanup();

    void testLayoutConstruction();

private:
    void checkArea1(Sublime::MainWindow *mw);

    Sublime::Controller *m_controller;
    Sublime::IdealCentralWidget* m_cw;

    Sublime::Area *m_area1;
    Sublime::Area *m_area2;
};

#endif

