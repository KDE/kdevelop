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
#ifndef SUBLIMEAREAPRINTER_H
#define SUBLIMEAREAPRINTER_H

#include <QString>
#include <sublime/sublimedefs.h>

namespace Sublime {
    class AreaIndex;
    class View;
}

//those two classes will pretty-print area views and toolviews
//make sure you provided object names for your views (with setObjectName())

struct AreaViewsPrinter {
    AreaViewsPrinter();
    bool operator()(Sublime::AreaIndex *index);
    QString result;
};

struct AreaToolViewsPrinter {
    AreaToolViewsPrinter();
    bool operator()(Sublime::View *view, Sublime::Position position);
    QString result;
};

#endif

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
