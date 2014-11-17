/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright 2014 Michael Ferris <mike.ferrisb@gmail.com>                *
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

#ifndef SIMPLEREFACTORING_H
#define SIMPLEREFACTORING_H

#include <interfaces/context.h>
#include <language/codegen/basicrefactoring.h>

namespace KDevelop {
class ContextMenuExtension;
class Declaration;
}

class SimpleRefactoring : public KDevelop::BasicRefactoring
{
  Q_OBJECT

public:
    explicit SimpleRefactoring(QObject* parent = 0);

    void fillContextMenu(KDevelop::ContextMenuExtension& extension, KDevelop::Context* context);
};

#endif
