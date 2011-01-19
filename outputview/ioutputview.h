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

#include <QtCore/QFlags>
#include <KDE/KIcon>

class QString;
class QAbstractItemModel;
class QModelIndex;
class QAbstractItemDelegate;

/**
@author Andreas Pakulat
*/
namespace KDevelop
{

class IOutputView
{
public:

    enum Behaviour
    {
        AllowUserClose = 0x1 /**< allow the user to close the view */,
        AlwaysShowView = 0x2 /**< always show the view */,
        AutoScroll     = 0x4 /**< automatically scroll the view */,
        ShowItemsButton= 0x8
    };
    Q_DECLARE_FLAGS(Behaviours, Behaviour)

    enum ViewType
    {
        OneView      = 0 /**< there's only one outputview, newly registered outputs will replace existing ones */,
        HistoryView  = 1 /**< The toolview will have a history with forward/backward buttons */,
        MultipleView = 2 /**< show multiples outputs in a toolview at the same time */
    };

    enum StandardToolView
    {
        BuildView = 0 /**< the standard outputview for building output */,
        RunView =   1 /**< the standard outputview for running apps */,
        DebugView = 2 /**< the standard outputview for debugging apps */,
        TestView  = 4 /**< the standard outputview for verbose test output */,
        VcsView   = 8 /**< the standard outputview for VCS commands */
    };

    virtual ~IOutputView() {}

    /**
     * fetch the identifier for one of the standard toolviews
     * This will automatically create the toolview if it doesn't exist yet
     * @param view the standard toolview to get the identifier for
     * @returns the identifier for the standard toolview
     */
    virtual int standardToolView( StandardToolView view ) = 0;

    /**
     * Register a new toolview for output with the given title, behaviour and type
     * If there already exists a toolview with this title and type return the existing id
     * @param title the Title to be displayed on the toolview
     * @param type the type of view that should be created
     * @returns an toolview id that identifies the new view and is used in the other
     *          methods
     */
    virtual int registerToolView( const QString& title, ViewType type = OneView, const KIcon& icon = KIcon() ) = 0;

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

    virtual void scrollOutputTo( int id, const QModelIndex& ) = 0;

    enum Ownership {
        KeepOwnership,
        TakeOwnership
    };

    /**
     * Sets the model of the registered output identified by id to model
     * Does nothing if the id doesn't exist
     *
     * \param takeOwnership If true, the output view plugin takes ownership of the model,
     *                      and deletes it when the view is removed.  If false, the ownership
     *                      remains with the caller.
     *
     */
    virtual void setModel( int id, QAbstractItemModel* model, Ownership takeOwnership = KeepOwnership ) = 0;

    /**
     * Sets the item delegate of the registered output identified by id to @p delegate
     * Does nothing if the id doesn't exist
     *
     * \param takeOwnership If true, the output view plugin takes ownership of the model,
     *                      and deletes it when the view is removed.  If false, the ownership
     *                      remains with the caller.
     *
     */
    virtual void setDelegate( int id, QAbstractItemDelegate* model, Ownership takeOwnership = KeepOwnership ) = 0;

    /**
     * remove a toolview, don't forget to emit toolViewRemoved when you implement this
     *
     * @param id identifies the view to remove
     */
    virtual void removeToolView( int id ) = 0;

    /**
     * remove an output view from a toolview. Don't forget to emit outputRemoved
     * when you implement this.
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
Q_DECLARE_INTERFACE( KDevelop::IOutputView, "org.kdevelop.IOutputView" )

#endif

