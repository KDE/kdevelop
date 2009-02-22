/***************************************************************************
 *   Copyright 2006 Alexander Dymo <adymo@kdevelop.org>             *
 *   Copyright 2007 Andreas Pakulat <apaku@gmx.de>                  *
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

K_PLUGIN_FACTORY(DocumentSwitcherFactory, registerPlugin<DocumentSwitcherPlugin>(); )
K_EXPORT_PLUGIN(DocumentSwitcherFactory(KAboutData("kdevdocumentswitcher","kdevdocumentswitcher",ki18n("Document Switcher"), "0.1", ki18n("Switch between open documents using most-recently-used list"), KAboutData::License_GPL)))


DocumentSwitcherPlugin::DocumentSwitcherPlugin(QObject *parent, const QVariantList &/*args*/)
    :KDevelop::IPlugin(DocumentSwitcherFactory::componentData(), parent), view(0)
{
    setXMLFile("kdevdocumentswitcher.rc");
    connect( KDevelop::ICore::self()->uiController()->controller(), SIGNAL( mainWindowAdded( Sublime::MainWindow* ) ), SLOT( addMainWindow( Sublime::MainWindow* ) ) );
    addMainWindow( dynamic_cast<Sublime::MainWindow*>( KDevelop::ICore::self()->uiController()->activeMainWindow() ) );
    
    KAction* action = actionCollection()->addAction ( "last_used_documents_forward" );
    action->setText( "Last View" );
    action->setShortcut( Qt::CTRL | Qt::Key_Tab );
    action->setWhatsThis( "<b>Walk through last used Documents</b><br/>Opens a list to walk through the list of last used documents." );
    action->setStatusTip( "Walk through the list of last used documents" );
    connect( action, SIGNAL(triggered()), SLOT(walkForward()) );
    
    action = actionCollection()->addAction ( "last_used_documents_backward" );
    action->setText( "Previous View" );
    action->setShortcut( Qt::CTRL | Qt::SHIFT | Qt::Key_Tab );
    action->setWhatsThis( "<b>Walk through last used Documents (Backward)</b><br/>Opens a list to walk through the list of last used documents." );
    action->setStatusTip( "Walk through the list of last used documents" );
    connect( action, SIGNAL(triggered()), SLOT(walkBackward()) );
    
}

void DocumentSwitcherPlugin::walkForward()
{
    Sublime::MainWindow* window = dynamic_cast<Sublime::MainWindow*>( KDevelop::ICore::self()->uiController()->activeMainWindow() );
    if( !window || !documentLists.contains( window ) || !documentLists[window].contains( window->area() ) )
    {
        kWarning() << "This should not happen, tried to walk through document list of an unknown mainwindow!";
        return;
    }
    if( !view )
    {
        view = new QListView( KDevelop::ICore::self()->uiController()->activeMainWindow() );
        view->setSelectionBehavior( QAbstractItemView::SelectRows );
        view->setSelectionMode( QAbstractItemView::SingleSelection );
        view->setWindowFlags( Qt::Popup | Qt::FramelessWindowHint );
        connect( view, SIGNAL(clicked(const QModelIndex&)), SLOT(switchToView(const QModelIndex&)) );
        connect( view, SIGNAL(activated(const QModelIndex&)), SLOT(switchToView(const QModelIndex&)) );
        
        QStringList views;
        foreach( Sublime::View* v, documentLists[window][window->area()] )
        {
            views << v->document()->title();
        }
        view->setModel( new QStringListModel( views, view ) );
        view->move( QCursor::pos() );
        view->show();
    }
}

void DocumentSwitcherPlugin::walkBackward()
{
}

DocumentSwitcherPlugin::~DocumentSwitcherPlugin()
{
}

void DocumentSwitcherPlugin::switchToView( const QModelIndex& idx )
{
    kDebug() << "switching to idx";
    view->hide();
    view->deleteLater();
    view = 0;
}

void DocumentSwitcherPlugin::switchToView()
{
    kDebug() << "Switching to view";
    view->hide();
    view->deleteLater();
    view = 0;
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
    kDebug() << "got new mainwindow";
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
    kDebug() << "removing mainwindow" << mainwindow;
    disconnect( mainwindow, 0, this, 0 );
    documentLists.remove( mainwindow );
}


void DocumentSwitcherPlugin::changeArea( Sublime::Area* area ) 
{
    Sublime::MainWindow* mainwindow = qobject_cast<Sublime::MainWindow*>( sender() );
    Q_ASSERT( mainwindow );
    kDebug() << "changing area" << area;
    if( !documentLists[mainwindow].contains( area ) )
    {
        kDebug() << "storing view list";
        storeAreaViewList( mainwindow, area );
    }
}
void DocumentSwitcherPlugin::changeView( Sublime::View* view )
{
    if( !view )
        return;
        
    Sublime::MainWindow* mainwindow = qobject_cast<Sublime::MainWindow*>( sender() );
    Q_ASSERT( mainwindow );
    
    kDebug() << "changing view:" << view << view->document() << view->document()->title();
    
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
    kDebug() << "removing view:" << view << view->document() << view->document()->title();
    
    Sublime::Area* area = mainwindow->area();
    
    int idx = documentLists[mainwindow][area].indexOf( view );
    if( idx != -1 )
    {
        documentLists[mainwindow][area].removeAt( idx );
    }
}


#include "documentswitcherplugin.moc"

