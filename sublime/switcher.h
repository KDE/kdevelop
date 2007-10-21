/***************************************************************************
 *   Copyright 2006-2007 Alexander Dymo  <adymo@kdevelop.org>       *
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
#ifndef SUBLIMESWITCHER_H
#define SUBLIMESWITCHER_H

#include <QtGui/QComboBox>

#include <sublimeexport.h>

namespace Sublime {

/**
@short Switcher for views inside the one container.
*/
class SUBLIME_EXPORT Switcher: public QComboBox {
Q_OBJECT
public:
    Switcher(QWidget *parent = 0);
    ~Switcher();
private:
    class SwitcherPrivate* const d;
};

}

#endif

