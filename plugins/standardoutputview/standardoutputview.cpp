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

#include <icore.h>
#include <iuicontroller.h>

#include <kpluginfactory.h>
#include <kpluginloader.h>
#include <klocale.h>
#include <kdebug.h>
#include <kactioncollection.h>
#include <kaction.h>

#include <sublime/view.h>
#include <sublime/area.h>
#include <sublime/controller.h>

#include "toolviewdata.h"

K_PLUGIN_FACTORY(StandardOutputViewFactory, registerPlugin<StandardOutputView>(); )
K_EXPORT_PLUGIN(StandardOutputViewFactory("kdevstandardoutputview"))


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
    action->setText("Next");
    action->setShortcut( QKeySequence(Qt::Key_F4) );
    connect(action, SIGNAL(triggered(bool)), this, SIGNAL(selectNextItem()));

    action = actionCollection()->addAction("prev_error");
    action->setText("Previous");
    action->setShortcut( QKeySequence(Qt::SHIFT | Qt::Key_F4) );
    connect(action, SIGNAL(triggered(bool)), this, SIGNAL(selectPrevItem()));

    connect(KDevelop::ICore::self()->uiController()->controller(), SIGNAL(aboutToRemoveView(View*)), this, SLOT(removeSublimeView(View*)));

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
            return registerToolView( i18n("Build"), KDevelop::IOutputView::MultipleView );
            break;
        }
        case KDevelop::IOutputView::RunView:
        {
            return registerToolView( i18n("Run"), KDevelop::IOutputView::MultipleView );
            break;
        }
    }
}

int StandardOutputView::registerToolView( const QString& title,
                                          KDevelop::IOutputView::ViewType type )
{
    int newid = -1;
    if( ids.isEmpty() )
        newid = 0;
    else
    {
        foreach( ToolViewData* d, toolviews.values() )
        {
            if( d->title == title )
                return d->toolViewId;
        }
        newid = ids.last()+1;
    }
    kDebug(9500) << "Registering view" << title << "with type:" << type;
    ToolViewData* tvdata = new ToolViewData( this );
    tvdata->toolViewId = newid;
    tvdata->type = type;
    tvdata->title = title;
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
            Sublime::View* v = toolviews.value( _id )->views.at(0);
            OutputWidget* w = qobject_cast<OutputWidget*>( v->widget() );
            w->raiseOutput( id );
            v->requestRaise();
        }
    }
}

void StandardOutputView::setModel( int id, QAbstractItemModel* model )
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
        kDebug(9500) << "Trying to set model on unknown view-id:" << id;
    else
    {
        toolviews.value( tvid )->outputdata.value( id )->setModel( model );
    }
}

void StandardOutputView::setDelegate( int id, QAbstractItemDelegate* delegate )
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
        kDebug(9500) << "Trying to set model on unknown view-id:" << id;
    else
    {
        toolviews.value( tvid )->outputdata.value( id )->setDelegate( delegate );
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
            foreach( Sublime::Area* area, KDevelop::ICore::self()->uiController()->controller()->areas() )
            {
                area->removeToolView( view );
            }
        }
        delete td;
        toolviews.remove(id);
        emit toolViewRemoved(id);
    }
}

void StandardOutputView::removeOutput( int id )
{
    foreach( ToolViewData* td, toolviews.values() )
    {
        if( td->outputdata.contains( id ) )
        {
            foreach( Sublime::View* view, td->views )
            {
                OutputWidget* widget = qobject_cast<OutputWidget*>( view->widget() );
                widget->removeOutput( id );
            }
            emit outputRemoved( td->toolViewId, id );
        }
    }
}

#include "standardoutputview.moc"
