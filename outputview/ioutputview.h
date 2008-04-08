/* KDevelop Output View
 *
 * Copyright 2006-2007 Andreas Pakulat <apaku@gmx.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#ifndef KDEVIOUTPUTVIEW_H
#define KDEVIOUTPUTVIEW_H

#include <interfaces/iextension.h>

#include <QFlags>

class QString;
class QAbstractItemModel;
class QAbstractItemDelegate;

/**
@author Andreas Pakulat
*/
namespace KDevelop
{

class IOutputView
{
public:

    enum Behaviour {
        AllowUserClose = 0x1 /**< allow the user to close the view */,
        AlwaysShowView = 0x2 /**< always show the view */,
        AutoScroll     = 0x4 /**< automatically scroll the view */
    };
    Q_DECLARE_FLAGS(Behaviours, Behaviour)

    virtual ~IOutputView() {}

    /**
     * Register a new view with the given title
     * @param title the Title to be displayed on the view-switcher
     * @returns an id that identifies the new view and is used in the other
     *          methods
     */
    virtual int registerView( const QString& title, Behaviours ) = 0;

    /**
     * Raise a given view
     */
    virtual void raiseView( int id ) = 0;

    /**
     * Sets the model of the registered view identified by id to model
     *
     * This function does _not_ take ownership of the model, the user is
     * responsible for deleting the model once the view is removed
     *
     * Does nothing if the id doesn't exist
     */
    virtual void setModel( int id, QAbstractItemModel* model ) = 0;

    /**
     * Sets the item delegate of the registered view identified by id to @p delegate
     *
     * This function does _not_ take ownership of the delegate, the user is
     * responsible for deleting the delegate once the view is removed
     *
     * Does nothing if the id doesn't exist
     */
    virtual void setDelegate( int id, QAbstractItemDelegate* model ) = 0;

    /**
     * remove a view, don't forget to emit viewRemoved when you implement this
     *
     * @param id identifies the view to remove
     */
    virtual void removeView( int id ) = 0;

Q_SIGNALS:
    /**
     * emitted after a view was removed
     *
     * @param id identifies the removed view
     */
    void viewRemoved( int id );
};

Q_DECLARE_OPERATORS_FOR_FLAGS(IOutputView::Behaviours)

}
KDEV_DECLARE_EXTENSION_INTERFACE_NS( KDevelop, IOutputView, "org.kdevelop.IOutputView" )
Q_DECLARE_INTERFACE( KDevelop::IOutputView, "org.kdevelop.IOutputView" )

#endif

