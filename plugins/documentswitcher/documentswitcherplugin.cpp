/***************************************************************************
 *   Copyright 2009 Andreas Pakulat <apaku@gmx.de>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/
#include "documentswitcherplugin.h"

#include <QListView>
#include <QStringListModel>

#include <klocale.h>
#include <kpluginfactory.h>
#include <kpluginloader.h>
#include <kdebug.h>
#include <kactioncollection.h>
#include <kaboutdata.h>

#include <interfaces/icore.h>
#include <interfaces/iuicontroller.h>
#include <sublime/controller.h>
#include <sublime/view.h>
#include <sublime/area.h>
#include <sublime/mainwindow.h>
#include <sublime/document.h>

#include "documentswitchertreeview.h"

K_PLUGIN_FACTORY(DocumentSwitcherFactory, registerPlugin<DocumentSwitcherPlugin>(); )
K_EXPORT_PLUGIN(DocumentSwitcherFactory(KAboutData("kdevdocumentswitcher","kdevdocumentswitcher",ki18n("Document Switcher"), "0.1", ki18n("Switch between open documents using most-recently-used list"), KAboutData::License_GPL)))

//TODO: Disable actions if list of current mainwindow < 2
//TODO: Show frame around view's widget while walking through
//TODO: Make the widget transparent
//TODO: Better placement, at cursor position might not be ideal, maybe on the right side of the central widget
//BUG: First document after start is not in list, seems to be caused by setting the stringlist

DocumentSwitcherPlugin::DocumentSwitcherPlugin(QObject *parent, const QVariantList &/*args*/)
    :KDevelop::IPlugin(DocumentSwitcherFactory::componentData(), parent), view(0)
{
    setXMLFile("kdevdocumentswitcher.rc");
    connect( KDevelop::ICore::self()->uiController()->controller(), SIGNAL( mainWindowAdded( Sublime::MainWindow* ) ), SLOT( addMainWindow( Sublime::MainWindow* ) ) );
    addMainWindow( dynamic_cast<Sublime::MainWindow*>( KDevelop::ICore::self()->uiController()->activeMainWindow() ) );
    
    KAction* action = actionCollection()->addAction ( "last_used_views_forward" );
    action->setText( i18n( "Next View" ) );
    action->setShortcut( Qt::CTRL | Qt::Key_Tab );
    action->setWhatsThis( i18n( "<b>Walk through last used Views</b><br/>Opens a list to walk through the list of last used views." ) );
    action->setStatusTip( i18n( "Walk through the list of last used views" ) );
    connect( action, SIGNAL(triggered()), SLOT(walkForward()) );
    
    action = actionCollection()->addAction ( "last_used_views_backward" );
    action->setText( i18n( "Previous View" ) );
    action->setShortcut( Qt::CTRL | Qt::SHIFT | Qt::Key_Tab );
    action->setWhatsThis( i18n( "<b>Walk through last used Views (Reverse)</b><br/>Opens a list to walk through the list of last used views in reverse." ) );
    action->setStatusTip( i18n( "Walk through the list of last used views" ) );
    connect( action, SIGNAL(triggered()), SLOT(walkBackward()) );
    
    view = new DocumentSwitcherTreeView( this, KDevelop::ICore::self()->uiController()->activeMainWindow() );
    view->setSelectionBehavior( QAbstractItemView::SelectRows );
    view->setSelectionMode( QAbstractItemView::SingleSelection );
    view->setWindowFlags( Qt::Popup | Qt::FramelessWindowHint );
    view->addAction( actionCollection()->action( "last_used_views_forward" ) );
    view->addAction( actionCollection()->action( "last_used_views_backward" ) );
    connect( view, SIGNAL(clicked(const QModelIndex&)), SLOT(switchToView(const QModelIndex&)) );
    connect( view, SIGNAL(activated(const QModelIndex&)), SLOT(switchToView(const QModelIndex&)) );
    
    model = new QStringListModel( view );
    view->setModel( model );    
}

void DocumentSwitcherPlugin::walkForward()
{
    Sublime::MainWindow* window = dynamic_cast<Sublime::MainWindow*>( KDevelop::ICore::self()->uiController()->activeMainWindow() );
    if( !window || !documentLists.contains( window ) || !documentLists[window].contains( window->area() ) )
    {
        kWarning() << "This should not happen, tried to walk through document list of an unknown mainwindow!";
        return;
    }
    QModelIndex idx;
    if( !view->isVisible() )
    {
        QStringList views;
        foreach( Sublime::View* v, documentLists[window][window->area()] )
        {
            views << v->document()->title();
        }
    
        model->setStringList( views );
        view->move( QCursor::pos() );
        idx = model->index( 1, 0 );
        view->show();
    } else 
    {
        int newrow = view->selectionModel()->currentIndex().row() + 1;
        if( newrow == model->rowCount() ) 
        {
            newrow = 0;
        }
        idx = model->index( newrow, 0 );
    }
    view->selectionModel()->select( idx, QItemSelectionModel::ClearAndSelect );
    view->selectionModel()->setCurrentIndex( idx, QItemSelectionModel::SelectCurrent );
}

void DocumentSwitcherPlugin::walkBackward()
{
    Sublime::MainWindow* window = dynamic_cast<Sublime::MainWindow*>( KDevelop::ICore::self()->uiController()->activeMainWindow() );
    if( !window || !documentLists.contains( window ) || !documentLists[window].contains( window->area() ) )
    {
        kWarning() << "This should not happen, tried to walk through document list of an unknown mainwindow!";
        return;
    }
    QModelIndex idx;
    if( !view->isVisible() )
    {
        QStringList views;
        foreach( Sublime::View* v, documentLists[window][window->area()] )
        {
            views << v->document()->title();
        }
    
        model->setStringList( views );
        view->move( QCursor::pos() );
        idx = model->index( model->rowCount()-1, 0 );
        view->show();
    } else 
    {
        int newrow = view->selectionModel()->currentIndex().row() - 1;
        if( newrow == -1 ) 
        {
            newrow = model->rowCount()-1;
        }
        idx = model->index( newrow, 0 );
    }
    view->selectionModel()->select( idx, QItemSelectionModel::ClearAndSelect );
    view->selectionModel()->setCurrentIndex( idx, QItemSelectionModel::SelectCurrent );
}

DocumentSwitcherPlugin::~DocumentSwitcherPlugin()
{
}

void DocumentSwitcherPlugin::switchToView( const QModelIndex& idx )
{
    int row = view->selectionModel()->selectedRows().first().row();
    view->hide();
    
    Sublime::MainWindow* window = dynamic_cast<Sublime::MainWindow*>( KDevelop::ICore::self()->uiController()->activeMainWindow() );
    if( window && documentLists.contains( window ) && documentLists[window].contains( window->area() ) )
    {
        const QList<Sublime::View*> l = documentLists[window][window->area()];
        if( row >= 0 && row < l.size() )
        {
            window->activateView( l.at( row ) );
        }
    }
}

void DocumentSwitcherPlugin::unload()
{
    foreach( Sublime::MainWindow* mw, documentLists.keys() )
    {
        removeMainWindow( mw );
    }
}


void DocumentSwitcherPlugin::storeAreaViewList( Sublime::MainWindow* mainwindow, Sublime::Area* area )
{
    if( !documentLists.contains( mainwindow ) || !documentLists[mainwindow].contains(area) ) 
    {
        QMap<Sublime::Area*, QList<Sublime::View*> > areas;
        areas.insert( area, area->views() );
        documentLists.insert( mainwindow, areas );
    }
    
}

void DocumentSwitcherPlugin::addMainWindow( Sublime::MainWindow* mainwindow ) 
{
    if( !mainwindow ) 
        return;
    storeAreaViewList( mainwindow, mainwindow->area() );
    connect( mainwindow, SIGNAL(areaChanged(Sublime::Area*)), SLOT(changeArea(Sublime::Area*)) );
    connect( mainwindow, SIGNAL(activeViewChanged(Sublime::View*)), SLOT(changeView(Sublime::View*)) );
    connect( mainwindow, SIGNAL(aboutToRemoveView(Sublime::View*)), SLOT(removeView(Sublime::View*)) );
    connect( mainwindow, SIGNAL(destroyed(QObject*)), SLOT(removeMainWindow(QObject*)));
}

void DocumentSwitcherPlugin::removeMainWindow( QObject* obj ) 
{
    Sublime::MainWindow* mainwindow = qobject_cast<Sublime::MainWindow*>( obj );
    if( !mainwindow )
        return;
    disconnect( mainwindow, 0, this, 0 );
    documentLists.remove( mainwindow );
}


void DocumentSwitcherPlugin::changeArea( Sublime::Area* area ) 
{
    Sublime::MainWindow* mainwindow = qobject_cast<Sublime::MainWindow*>( sender() );
    Q_ASSERT( mainwindow );
    if( !documentLists[mainwindow].contains( area ) )
    {
        storeAreaViewList( mainwindow, area );
    }
}
void DocumentSwitcherPlugin::changeView( Sublime::View* view )
{
    if( !view )
        return;
        
    Sublime::MainWindow* mainwindow = qobject_cast<Sublime::MainWindow*>( sender() );
    Q_ASSERT( mainwindow );
    
    Sublime::Area* area = mainwindow->area();
    
    int idx = documentLists[mainwindow][area].indexOf( view );
    if( idx != -1 )
    {
        documentLists[mainwindow][area].removeAt( idx );
    }
    documentLists[mainwindow][area].prepend( view );
}

void DocumentSwitcherPlugin::removeView( Sublime::View* view ) 
{
    if( !view )
        return;
        
    Sublime::MainWindow* mainwindow = qobject_cast<Sublime::MainWindow*>( sender() );
    Q_ASSERT( mainwindow );
    
    Sublime::Area* area = mainwindow->area();
    
    int idx = documentLists[mainwindow][area].indexOf( view );
    if( idx != -1 )
    {
        documentLists[mainwindow][area].removeAt( idx );
    }
}


#include "documentswitcherplugin.moc"

