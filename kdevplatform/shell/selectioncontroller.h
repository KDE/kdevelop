/*
    SPDX-FileCopyrightText: 2009 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_SELECTIONCONTROLLER_H
#define KDEVPLATFORM_SELECTIONCONTROLLER_H

#include <interfaces/iselectioncontroller.h>
#include "shellexport.h"

namespace KDevelop
{

class Context;
class SelectionControllerPrivate;

/**
 * The main controller for running processes.
 */
class KDEVPLATFORMSHELL_EXPORT SelectionController : public ISelectionController
{
    Q_OBJECT
public:
    ///Constructor.
    explicit SelectionController(QObject *parent);
    ~SelectionController() override;

    /**
     * Provides the current selection, note that this might be 0
     */
    Context* currentSelection() override;


    void initialize();
    void cleanup();

public Q_SLOTS:
    /**
     * updates the current selection
     * @param context the new selection
     */
    void updateSelection( Context* context ) override;

private:
    const QScopedPointer<class SelectionControllerPrivate> d_ptr;
    Q_DECLARE_PRIVATE(SelectionController)
};

}

#endif // KDEVPLATFORM_SELECTIONCONTROLLER_H
