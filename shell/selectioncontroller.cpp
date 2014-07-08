/* This file is part of KDevelop
Copyright 2009 Andreas Pakulat <apaku@gmx.de>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public License
along with this library; see the file COPYING.LIB.  If not, write to
the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
Boston, MA 02110-1301, USA.
*/

#include "selectioncontroller.h"

#include <interfaces/context.h>

namespace KDevelop
{

class SelectionControllerPrivate
{
public:
    Context* currentSelection;
};

SelectionController::SelectionController( QObject* o )
    : ISelectionController( o ), d(new SelectionControllerPrivate)
{
    d->currentSelection = 0;
}

SelectionController::~SelectionController()
{
    delete d;
}

Context* SelectionController::currentSelection()
{
    return d->currentSelection;
}

void SelectionController::updateSelection( Context* ctx )
{
    d->currentSelection = ctx;
    emit selectionChanged( d->currentSelection );
}

void SelectionController::initialize()
{
}

void SelectionController::cleanup()
{
}

}

