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
    QScopedPointer<Context> currentSelection;
};

SelectionController::SelectionController( QObject* o )
    : ISelectionController(o)
    , d_ptr(new SelectionControllerPrivate)
{
}

SelectionController::~SelectionController() = default;

Context* SelectionController::currentSelection()
{
    Q_D(SelectionController);

    return d->currentSelection.data();
}

void SelectionController::updateSelection( Context* ctx )
{
    Q_D(SelectionController);

    d->currentSelection.reset(ctx);
    emit selectionChanged(d->currentSelection.data());
}

void SelectionController::initialize()
{
}

void SelectionController::cleanup()
{
}

}
