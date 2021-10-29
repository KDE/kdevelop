/*
    SPDX-FileCopyrightText: 2009 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
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
