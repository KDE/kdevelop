/*
    SPDX-FileCopyrightText: 2006-2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_IOUTPUTVIEW_H
#define KDEVPLATFORM_IOUTPUTVIEW_H

#include "outputviewexport.h"

#include <util/namespacedoperatorbitwiseorworkaroundqtbug.h>

#include <QFlags>
#include <QIcon>
#include <QMetaType>
#include <QtPlugin>

class QString;
class QAbstractItemModel;
class QModelIndex;
class QAbstractItemDelegate;
class QAction;

/**
@author Andreas Pakulat
*/
namespace KDevelop
{

class KDEVPLATFORMOUTPUTVIEW_EXPORT IOutputView
{
public:

    enum Behaviour
    {
        AllowUserClose = 0x1 /**< allow the user to close the view */,
        AlwaysShowView = 0x2 /**< always show the view */,
        AutoScroll     = 0x4 /**< automatically scroll the view */
    };
    Q_DECLARE_FLAGS(Behaviours, Behaviour)

    enum Option
    {
        NoOptions = 0x0,
        ShowItemsButton = 0x1 /**< show the two buttons (select and focus) */,
        AddFilterAction = 0x2 /**< add a filter action */
    };
    Q_DECLARE_FLAGS(Options, Option)

    enum ViewType
    {
        OneView      = 0 /**< there's only one outputview, newly registered outputs will replace existing ones */,
        HistoryView  = 1 /**< The tool view will have a history with forward/backward buttons */,
        MultipleView = 2 /**< show multiples outputs in a tool view at the same time */
    };

    enum StandardToolView
    {
        BuildView = 0 /**< the standard outputview for building output */,
        RunView =   1 /**< the standard outputview for running apps */,
        DebugView = 2 /**< the standard outputview for debugging apps */,
        TestView  = 4 /**< the standard outputview for verbose test output */,
        VcsView   = 8 /**< the standard outputview for VCS commands */
    };

    virtual ~IOutputView();

    /**
     * Fetch the identifier for one of the standard tool views.
     * This will automatically create the tool view if it doesn't exist yet
     * @param view the standard tool view to get the identifier for
     * @returns the identifier for the standard tool view
     */
    virtual int standardToolView( StandardToolView view ) = 0;

    /**
     * Register a new tool view for output with the given title, behaviour and type.
     * If there already exists a tool view with this title and type return the existing id
     * @param configSubgroupName the name of the config group used to save settings related to the view.
     *                           An empty name disables settings for the tool view.
     * @param title the Title to be displayed on the tool view
     * @param type the type of view that should be created
     * @param icon the icon of the tool view
     * @param option the options of the tool view
     * @param actionList list of actions adding to the toolbar
     * @returns an tool view id that identifies the new view and is used in the other
     *          methods
     */
    virtual int registerToolView(const QString& configSubgroupName, const QString& title, ViewType type = OneView,
                                 const QIcon& icon = QIcon(), Options option = ShowItemsButton,
                                 const QList<QAction*>& actionList = QList<QAction*>()) = 0;

    /**
     * Register a new output view in a given tool view. How this new view is created depends
     * on the type of the tool view.
     * @param toolViewId the id of the tool view, created by registerToolView
     * @param title the title to use for the new output in the tool view
     * @param behaviour the Behaviour of the output
     * @returns the id of the output to supply to the other methods
     */
    virtual int registerOutputInToolView( int toolViewId, const QString& title,
                                          Behaviours behaviour = AllowUserClose ) = 0;

    /**
     * Raise a given view
     */
    virtual void raiseOutput( int outputId ) = 0;

    virtual void scrollOutputTo( int outputId, const QModelIndex& ) = 0;

    /**
     * Sets the model of the registered output identified by @p outputId to @p model.
     *
     * Does nothing if the id doesn't exist. The output view takes ownership of the model.
     *
     * NOTE: Do not reuse the same model for different views.
     */
    virtual void setModel( int outputId, QAbstractItemModel* model ) = 0;
    /**
     * Sets the item delegate of the registered output identified by @p outputId to @p delegate.
     *
     * Does nothing if the id doesn't exist. The output view takes ownership of the delegate.
     *
     * NOTE: Do not reuse the same delegate for different views.
     */
    virtual void setDelegate( int outputId, QAbstractItemDelegate* model ) = 0;

    /**
     * Sets a @p title for the specified @p outputIds
     */
    virtual void setTitle( int outputId, const QString& title ) = 0;

    /**
     * Remove a tool view, don't forget to emit toolViewRemoved when you implement this.
     *
     * @param toolViewId identifies the view to remove
     */
    virtual void removeToolView(int toolViewId) = 0;

    /**
     * Remove an output view from a tool view. Don't forget to emit outputRemoved
     * when you implement this.
     * @param outputId the id of the outputview to remove
     */
    virtual void removeOutput( int outputId ) = 0;

Q_SIGNALS:
    /**
     * Emitted after a tool view was removed.
     *
     * @param toolViewId identifies the removed tool view
     */
    void toolViewRemoved(int toolViewId);

    /**
     * Emitted after a tool view was removed.
     *
     * @param toolViewId identifies the removed tool view
     * @param outputId identifies the removed output
     */
    void outputRemoved(int toolViewId, int outputId);
};

Q_DECLARE_OPERATORS_FOR_FLAGS(IOutputView::Behaviours)
Q_DECLARE_OPERATORS_FOR_FLAGS(IOutputView::Options)
} // namespace KDevelop

Q_DECLARE_METATYPE(KDevelop::IOutputView::StandardToolView)

Q_DECLARE_INTERFACE( KDevelop::IOutputView, "org.kdevelop.IOutputView" )

#endif
