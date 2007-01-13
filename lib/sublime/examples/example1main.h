/***************************************************************************
 *   Copyright (C) 2006-2007 by Alexander Dymo  <adymo@kdevelop.org>       *
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
#ifndef EXAMPLE1MAIN_H
#define EXAMPLE1MAIN_H

#include <kmainwindow.h>

namespace Sublime {
    class Area;
    class Controller;
}

class Example1Main: public KMainWindow {
    Q_OBJECT
public:
    Example1Main();

public slots:
    void selectArea1();
    void selectArea2();

private slots:
    void updateTitle(Sublime::Area *area);

private:
    Sublime::Controller *m_controller;

    Sublime::Area *m_area1;
    Sublime::Area *m_area2;

};

#endif

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
