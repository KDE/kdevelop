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

    enum ViewType {
        OneView      = 0x1 /**< there's only one outputview, newly registered outputs will replace existing ones */,
        HistoryView  = 0x2 /**< The toolview will have a history with forward/backward buttons */,
        MultipleView = 0x4 /**< show multiples outputs in a toolview at the same time */
    };

    virtual ~IOutputView() {}

    /**
     * Register a new toolview for output with the given title, behaviour and type
     * If there already exists a toolview with this title and type return the existing id
     * @param title the Title to be displayed on the toolview
     * @param type the type of view that should be created
     * @returns an toolview id that identifies the new view and is used in the other
     *          methods
     */
    virtual int registerToolView( const QString& title, ViewType type = OneView ) = 0;

    /**
     * Register a new output view in a given toolview. How this new view is created depends
     * on the type of the toolview.
     * @param toolviewId the id of the toolview, created by registerToolView
     * @param title the title to use for the new output in the toolview
     * @param behaviour the Behaviour of the output
     * @returns an id to supply to the other methods
     */
    virtual int registerOutputInToolView( int toolviewId, const QString& title, Behaviours behaviour = AllowUserClose ) = 0;

    /**
     * Raise a given view
     */
    virtual void raiseOutput( int id ) = 0;

    /**
     * Sets the model of the registered output identified by id to model
     *
     * This function does _not_ take ownership of the model, the user is
     * responsible for deleting the model once the view is removed
     *
     * Does nothing if the id doesn't exist
     */
    virtual void setModel( int id, QAbstractItemModel* model ) = 0;

    /**
     * Sets the item delegate of the registered output identified by id to @p delegate
     *
     * This function does _not_ take ownership of the delegate, the user is
     * responsible for deleting the delegate once the view is removed
     *
     * Does nothing if the id doesn't exist
     */
    virtual void setDelegate( int id, QAbstractItemDelegate* model ) = 0;

    /**
     * remove a toolview, don't forget to emit viewRemoved when you implement this
     *
     * @param id identifies the view to remove
     */
    virtual void removeToolView( int id ) = 0;

    /**
     * remove an output view from a toolview
     * @param toolviewId the id of the toolview containing the output
     * @param id the id of the outputview to remove
     */
    virtual void removeOutput( int id ) = 0;

Q_SIGNALS:
    /**
     * emitted after a toolview was removed
     *
     * @param id identifies the removed toolview
     */
    void toolViewRemoved( int id );

    /**
     * emitted after a toolview was removed
     *
     * @param toolviewId identifies the removed toolview
     * @param id identifies the removed output
     */
    void outputRemoved( int toolviewId, int id );
};

Q_DECLARE_OPERATORS_FOR_FLAGS(IOutputView::Behaviours)

}
KDEV_DECLARE_EXTENSION_INTERFACE_NS( KDevelop, IOutputView, "org.kdevelop.IOutputView" )
Q_DECLARE_INTERFACE( KDevelop::IOutputView, "org.kdevelop.IOutputView" )

#endif

