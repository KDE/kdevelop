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

#ifndef KDEVPLATFORM_ISELECTIONCONTROLLER_H
#define KDEVPLATFORM_ISELECTIONCONTROLLER_H

#include <QtCore/QObject>

#include "interfacesexport.h"

namespace KDevelop
{

class Context;

/**
 * The main controller for selection updates in the GUI.
 *
 * This controller can be used by selection consumers and selection providers.
 *
 * A selection provider (for example a treeview for the projects) should
 * call updateSelection() with an appropriate context filled with the details
 * of the selection
 *
 * A selection consumer who is interested in notifications when the selection has
 * been changed, should connect to the selectionChanged signal. The consumer should
 * retrieve all necessary information that it might need to do something with the
 * selection and store those. Storing the whole Context* might be dangerous as it
 * will be deleted on the next selection change.
 *
 */
class KDEVPLATFORMINTERFACES_EXPORT ISelectionController : public QObject
{
    Q_OBJECT
public:
    ///Constructor.
    ISelectionController(QObject *parent);
    ~ISelectionController();

    /**
     * Provides the current selection, note that this might be 0
     */
    virtual Context* currentSelection() = 0;

public Q_SLOTS:
    /**
     * updates the current selection.
     * The SelectionController takes ownership of the given context and
     * deletes it when the selection is updated the next time.
     * @param context the new selection
     */
    virtual void updateSelection( Context* context ) = 0;

Q_SIGNALS:
    /**
     * Notify that the current selection changed.
     * @note The context might be deleted behind the back (when a new selection
     * is set) of the receivers of this signal, so make sure to copy all the
     * needed information.
     */
    void selectionChanged( KDevelop::Context* );

};

}

#endif // KDEVPLATFORM_ISELECTIONCONTROLLER_H
