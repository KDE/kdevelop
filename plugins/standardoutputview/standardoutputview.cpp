/* KDevelop Standard OutputView
 *
 * Copyright 2006-2007 Andreas Pakulat <apaku@gmx.de>
 * Copyright 2007 Dukju Ahn <dukjuahn@gmail.com>
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

#include "standardoutputview.h"
#include "outputwidget.h"

#include <QtCore/QList>
#include <QtDesigner/QExtensionFactory>
#include <QtCore/QAbstractItemModel>
#include <QtGui/QAction>
#include <QtGui/QAbstractItemDelegate>

#include <kpluginfactory.h>
#include <kpluginloader.h>
#include <kaboutdata.h>
#include <klocale.h>
#include <kdebug.h>
#include <kactioncollection.h>
#include <kaction.h>

#include <interfaces/icore.h>
#include <interfaces/iuicontroller.h>

#include <sublime/view.h>
#include <sublime/area.h>
#include <sublime/controller.h>
#include <sublime/document.h>

#include "toolviewdata.h"

K_PLUGIN_FACTORY(StandardOutputViewFactory, registerPlugin<StandardOutputView>(); )
K_EXPORT_PLUGIN(StandardOutputViewFactory(KAboutData("kdevstandardoutputview","kdevstandardoutputview",ki18n("Output View"), "0.1", ki18n("Provides toolviews for presenting the output of running apps"), KAboutData::License_GPL)))


class OutputViewFactory : public KDevelop::IToolViewFactory{
public:
    OutputViewFactory(ToolViewData* data): m_data(data) {}
    virtual QWidget* create(QWidget *parent = 0)
    {
        return new OutputWidget( parent, m_data );
    }
    virtual Qt::DockWidgetArea defaultPosition()
    {
        return Qt::BottomDockWidgetArea;
    }
    virtual void viewCreated( Sublime::View* view )
    {
        m_data->views << view;
    }
    virtual QString id() const
    {
        return "org.kdevelop.OutputView";
    }
private:
    ToolViewData *m_data;
};

StandardOutputView::StandardOutputView(QObject *parent, const QVariantList &)
    : KDevelop::IPlugin(StandardOutputViewFactory::componentData(), parent)
{
    KDEV_USE_EXTENSION_INTERFACE( KDevelop::IOutputView )

    setXMLFile("kdevstandardoutputview.rc");
    // setup actions
    KAction *action;

    action = actionCollection()->addAction("next_error");
    action->setText(i18n("Jump to Next Outputmark"));
    action->setShortcut( QKeySequence(Qt::Key_F4) );
    connect(action, SIGNAL(triggered(bool)), this, SIGNAL(selectNextItem()));

    action = actionCollection()->addAction("prev_error");
    action->setText(i18n("Jump to Previous Outputmark"));
    action->setShortcut( QKeySequence(Qt::SHIFT | Qt::Key_F4) );
    connect(action, SIGNAL(triggered(bool)), this, SIGNAL(selectPrevItem()));

    connect(KDevelop::ICore::self()->uiController()->controller(), SIGNAL(aboutToRemoveView(Sublime::View*)), this, SLOT(removeSublimeView(Sublime::View*)));

}
    
void StandardOutputView::removeSublimeView( Sublime::View* v )
{
    foreach( ToolViewData* d, toolviews )
    {
        if( d->views.contains(v) )
        {
            if( d->views.count() == 1 )
            {
                toolviews.remove( d->toolViewId );
                ids.removeAll( d->toolViewId );
                delete d;
            } else
            {
                d->views.removeAll(v);
            }
        }
    }
}

StandardOutputView::~StandardOutputView()
{
}

int StandardOutputView::standardToolView( KDevelop::IOutputView::StandardToolView view )
{
    if( standardViews.contains( view ) )
    {
        return standardViews.value( view );
    }
    switch( view )
    {
        case KDevelop::IOutputView::BuildView:
        {
            return registerToolView( i18n("Build"), KDevelop::IOutputView::HistoryView, KIcon("run-build") );
            break;
        }
        case KDevelop::IOutputView::RunView:
        {
            return registerToolView( i18n("Run"), KDevelop::IOutputView::MultipleView, KIcon("system-run") );
            break;
        }
        case KDevelop::IOutputView::DebugView:
        {
            return registerToolView( i18n("Debug"), KDevelop::IOutputView::MultipleView, KIcon("debugger") );
            break;
        }
        case KDevelop::IOutputView::TestView:
        {
            return registerToolView( i18n("Test"), KDevelop::IOutputView::HistoryView, KIcon("system-run"));
            break;
        }
        case KDevelop::IOutputView::VcsView:
        {
            return registerToolView( i18n("Version Control"), KDevelop::IOutputView::HistoryView, KIcon("system-run"));
            break;
        }
    }

    return -1;
}

int StandardOutputView::registerToolView( const QString& title,
                                          KDevelop::IOutputView::ViewType type,
                                          const KIcon& icon)
{
    int newid = -1;
    if( ids.isEmpty() )
        newid = 0;
    else
    {
        foreach( ToolViewData* d, toolviews )
        {
            if( d->title == title )
                return d->toolViewId;
        }
        newid = ids.last()+1;
    }
    kDebug() << "Registering view" << title << "with type:" << type;
    ToolViewData* tvdata = new ToolViewData( this );
    tvdata->toolViewId = newid;
    tvdata->type = type;
    tvdata->title = title;
    tvdata->icon = icon;
    tvdata->plugin = this;
    core()->uiController()->addToolView( title, new OutputViewFactory( tvdata ) );
    ids << newid;
    toolviews[newid] = tvdata;
    return newid;
}

int StandardOutputView::registerOutputInToolView( int toolViewId,
                                                  const QString& title,
                                                  KDevelop::IOutputView::Behaviours behaviour )
{
    if( !toolviews.contains( toolViewId ) )
        return -1;
    int newid;
    if( ids.isEmpty() )
    {
        newid = 0;
    } else
    {
        newid = ids.last()+1;
    }
    ids << newid;
    toolviews.value( toolViewId )->addOutput( newid, title, behaviour );
    return newid;
}

void StandardOutputView::raiseOutput(int id)
{
    foreach( int _id, toolviews.keys() )
    {
        if( toolviews.value( _id )->outputdata.contains( id ) )
        {
            foreach( Sublime::View* v, toolviews.value( _id )->views ) {
                v->requestRaise();
            }
        }
    }
}

void StandardOutputView::setModel( int id, QAbstractItemModel* model, Ownership takeOwnership )
{
    int tvid = -1;
    foreach( int _id, toolviews.keys() )
    {
        if( toolviews.value( _id )->outputdata.contains( id ) )
        {
            tvid = _id;
            break;
        }
    }
    if( tvid == -1 )
        kDebug() << "Trying to set model on unknown view-id:" << id;
    else
    {
        toolviews.value( tvid )->outputdata.value( id )->setModel( model, takeOwnership == TakeOwnership );
    }
}

void StandardOutputView::setDelegate( int id, QAbstractItemDelegate* delegate, Ownership takeOwnership )
{
    int tvid = -1;
    foreach( int _id, toolviews.keys() )
    {
        if( toolviews.value( _id )->outputdata.contains( id ) )
        {
            tvid = _id;
            break;
        }
    }
    if( tvid == -1 )
        kDebug() << "Trying to set model on unknown view-id:" << id;
    else
    {
        toolviews.value( tvid )->outputdata.value( id )->setDelegate( delegate, takeOwnership == TakeOwnership );
    }
}

void StandardOutputView::removeToolView( int id )
{
    if( toolviews.contains(id) )
    {
        ToolViewData* td = toolviews.value(id);
        foreach( Sublime::View* view, td->views )
        {
            OutputWidget* widget = qobject_cast<OutputWidget*>( view->widget() );
            foreach( int outid, td->outputdata.keys() )
            {
                widget->removeOutput( outid );
            }
            foreach( Sublime::Area* area, KDevelop::ICore::self()->uiController()->controller()->allAreas() )
            {
                area->removeToolView( view );
            }
        }
        delete td;
        toolviews.remove(id);
        emit toolViewRemoved(id);
    }
}

OutputWidget* StandardOutputView::outputWidgetForId( int id ) const
{
    foreach( ToolViewData* td, toolviews )
    {
        if( td->outputdata.contains( id ) )
        {
            foreach( Sublime::View* view, td->views )
            {
                return qobject_cast<OutputWidget*>( view->widget() );
            }
        }
    }
    return 0;
}

void StandardOutputView::scrollOutputTo( int id, const QModelIndex& idx )
{
    OutputWidget* widget = outputWidgetForId( id );
    if( widget )
        widget->scrollToIndex( idx );
}

void StandardOutputView::removeOutput( int id )
{
    OutputWidget* widget = outputWidgetForId( id );
    if( widget ) 
    {
        widget->removeOutput( id );
    }
}

#include "standardoutputview.moc"
