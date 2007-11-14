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

#include "sublime/view.h"

K_PLUGIN_FACTORY(StandardOutputViewFactory, registerPlugin<StandardOutputView>(); )
K_EXPORT_PLUGIN(StandardOutputViewFactory("kdevstandardoutputview"))

class StandardOutputViewPrivate
{
public:
    class StandardOutputViewViewFactory* m_factory;
    QMap<int, QAbstractItemModel* > m_models;
    QMap<int, QAbstractItemDelegate* > m_delegates;
    QMap<int, QString> m_titles;
    QList<int> m_ids;
    QMap<int, KDevelop::IOutputView::CloseBehaviour> m_behaviours;
    QList<Sublime::View*> m_views;
};

class StandardOutputViewViewFactory : public KDevelop::IToolViewFactory{
public:
    StandardOutputViewViewFactory(StandardOutputView *part): m_part(part) {}
    virtual QWidget* create(QWidget *parent = 0)
    {
        Q_UNUSED(parent)
        OutputWidget* l = new OutputWidget( parent, m_part);
        QObject::connect( l, SIGNAL( viewRemoved( int ) ),
                 m_part, SIGNAL( viewRemoved( int ) ) );
        QObject::connect( m_part, SIGNAL( removeView( int ) ),
                          l, SLOT( removeView( int ) ) );
        QObject::connect( l, SIGNAL( viewRemoved( int ) ),
                          m_part, SLOT( removeViewData( int ) ) );
//         QObject::connect( l, SIGNAL( activated(const QModelIndex&) ),
//                  m_part, SIGNAL(activated(const QModelIndex&)) );
        QObject::connect( m_part, SIGNAL(selectNextItem()), l, SLOT(selectNextItem()) );
        QObject::connect( m_part, SIGNAL(selectPrevItem()), l, SLOT(selectPrevItem()) );
        return l;
    }
    virtual Qt::DockWidgetArea defaultPosition(const QString &/*areaName*/)
    {
        return Qt::BottomDockWidgetArea;
    }

    virtual void viewCreated(Sublime::View* view)
    {
        m_part->d->m_views << view;
    }
private:
    StandardOutputView *m_part;
};

StandardOutputView::StandardOutputView(QObject *parent, const QVariantList &)
    : KDevelop::IPlugin(StandardOutputViewFactory::componentData(), parent),
      d(new StandardOutputViewPrivate)
{
    KDEV_USE_EXTENSION_INTERFACE( KDevelop::IOutputView )
    d->m_factory = new StandardOutputViewViewFactory( this );
    core()->uiController()->addToolView( i18n("Output View"), d->m_factory );

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

}

StandardOutputView::~StandardOutputView()
{
    delete d;
}

int StandardOutputView::registerView( const QString& title,
                                          KDevelop::IOutputView::CloseBehaviour behaviour )
{
    int newid;
    if( d->m_ids.isEmpty() )
        newid = 0;
    else
        newid = d->m_ids.last()+1;
    kDebug(9500) << "Registering view" << title << "with behaviour:" << behaviour;
    d->m_ids << newid;
    d->m_titles[newid] = title;
    d->m_models[newid] = 0;
    d->m_behaviours[newid] = behaviour;

    foreach (Sublime::View* view, d->m_views)
        view->requestRaise();

    return newid;
}

KDevelop::IOutputView::CloseBehaviour StandardOutputView::closeBehaviour( int id ) const
{

    if( d->m_titles.contains( id ) )
    {
        return d->m_behaviours[id];
    }
    return KDevelop::IOutputView::AllowUserClose;
}

void StandardOutputView::setModel( int id, QAbstractItemModel* model )
{
    if( d->m_ids.contains( id ) )
    {
        d->m_models[id] = model;
        emit modelChanged( id );
    }else
        kDebug(9500) << "Trying to set model on unknown view-id:" << id;
}

void StandardOutputView::setDelegate( int id, QAbstractItemDelegate* delegate )
{
    if( d->m_ids.contains( id ) )
    {
        d->m_delegates[id] = delegate;
        emit delegateChanged( id );
    }else
        kDebug(9500) << "Trying to set delegate on unknown view-id:" << id;
}

QList<int> StandardOutputView::registeredViews() const
{
    return d->m_ids;
}

QAbstractItemModel* StandardOutputView::registeredModel( int id ) const
{
    if( d->m_models.contains( id ) )
    {
        return d->m_models[id];
    }
    return 0;
}

QAbstractItemDelegate* StandardOutputView::registeredDelegate( int id ) const
{
    if( d->m_delegates.contains( id ) )
    {
        return d->m_delegates[id];
    }
    return 0;
}

QString StandardOutputView::registeredTitle( int id ) const
{
    if( d->m_titles.contains( id ) )
    {
        return d->m_titles[id];
    }
    return QString();
}

void StandardOutputView::removeViewData( int id )
{
    if( d->m_models.contains( id ) )
    {
        d->m_models.remove(id);
    }
    if( d->m_delegates.contains( id ) )
    {
        d->m_delegates.remove( id );
    }
    if( d->m_titles.contains( id ) )
    {
        d->m_titles.remove( id );
    }
    if( d->m_behaviours.contains( id ) )
    {
        d->m_behaviours.remove( id );
    }
    if( d->m_ids.contains( id ) )
    {
        d->m_ids.removeAll( id );
    }

}

#include "standardoutputview.moc"
