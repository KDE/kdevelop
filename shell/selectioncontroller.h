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

#ifndef KDEVPLATFORM_SELECTIONCONTROLLER_H
#define KDEVPLATFORM_SELECTIONCONTROLLER_H

#include <interfaces/iselectioncontroller.h>
#include "shellexport.h"

namespace KDevelop
{

class Context;

/**
 * The main controller for running processes.
 */
class KDEVPLATFORMSHELL_EXPORT SelectionController : public ISelectionController
{
    Q_OBJECT
public:
    ///Constructor.
    SelectionController(QObject *parent);
    ~SelectionController();

    /**
     * Provides the current selection, note that this might be 0
     */
    virtual Context* currentSelection() override;


    void initialize();
    void cleanup();

public Q_SLOTS:
    /**
     * updates the current selection
     * @param context the new selection
     */
    virtual void updateSelection( Context* context ) override;
private:
    class SelectionControllerPrivate* const d;

};

}

#endif // KDEVPLATFORM_SELECTIONCONTROLLER_H
