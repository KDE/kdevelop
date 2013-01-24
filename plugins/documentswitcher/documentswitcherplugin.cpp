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
#include <QStandardItemModel>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/idocument.h>
#include <QDir>

K_PLUGIN_FACTORY(DocumentSwitcherFactory, registerPlugin<DocumentSwitcherPlugin>(); )
K_EXPORT_PLUGIN(DocumentSwitcherFactory(KAboutData("kdevdocumentswitcher","kdevdocumentswitcher",ki18n("Document Switcher"), "0.1", ki18n("Switch between open documents using most-recently-used list"), KAboutData::License_GPL)))

//TODO: Show frame around view's widget while walking through
//TODO: Make the widget transparent

DocumentSwitcherPlugin::DocumentSwitcherPlugin(QObject *parent, const QVariantList &/*args*/)
    :KDevelop::IPlugin(DocumentSwitcherFactory::componentData(), parent), view(0)
{
    setXMLFile("kdevdocumentswitcher.rc");
    kDebug() << "Adding active mainwindow from constructor" << KDevelop::ICore::self()->uiController()->activeMainWindow(); 
    addMainWindow( qobject_cast<Sublime::MainWindow*>( KDevelop::ICore::self()->uiController()->activeMainWindow() ) );
    connect( KDevelop::ICore::self()->uiController()->controller(), SIGNAL(mainWindowAdded(Sublime::MainWindow*)), SLOT(addMainWindow(Sublime::MainWindow*)) );
    
    forwardAction = actionCollection()->addAction ( "last_used_views_forward" );
    forwardAction->setText( i18n( "Last Used Views" ) );
    forwardAction->setIcon( KIcon("go-next-view-page") );
    forwardAction->setShortcut( Qt::CTRL | Qt::Key_Tab );
    forwardAction->setWhatsThis( i18n( "Opens a list to walk through the list of last used views." ) );
    forwardAction->setStatusTip( i18n( "Walk through the list of last used views" ) );
    connect( forwardAction, SIGNAL(triggered()), SLOT(walkForward()) );
    
    backwardAction = actionCollection()->addAction ( "last_used_views_backward" );
    backwardAction->setText( i18n( "Last Used Views (Reverse)" ) );
    backwardAction->setIcon( KIcon("go-previous-view-page") );
    backwardAction->setShortcut( Qt::CTRL | Qt::SHIFT | Qt::Key_Tab );
    backwardAction->setWhatsThis( i18n( "Opens a list to walk through the list of last used views in reverse." ) );
    backwardAction->setStatusTip( i18n( "Walk through the list of last used views" ) );
    connect( backwardAction, SIGNAL(triggered()), SLOT(walkBackward()) );
    
    view = new DocumentSwitcherTreeView( this );
    view->setSelectionBehavior( QAbstractItemView::SelectRows );
    view->setSelectionMode( QAbstractItemView::SingleSelection );
    view->addAction( forwardAction );
    view->addAction( backwardAction );
    connect( view, SIGNAL(clicked(QModelIndex)), SLOT(switchToView(QModelIndex)) );
    connect( view, SIGNAL(activated(QModelIndex)), SLOT(switchToView(QModelIndex)) );
    
    model = new QStandardItemModel( view );
    view->setModel( model );    
}

void DocumentSwitcherPlugin::walkForward()
{
    Sublime::MainWindow* window = qobject_cast<Sublime::MainWindow*>( KDevelop::ICore::self()->uiController()->activeMainWindow() );
    if( !window || !documentLists.contains( window ) || !documentLists[window].contains( window->area() ) )
    {
        kWarning() << "This should not happen, tried to walk through document list of an unknown mainwindow!";
        return;
    }
    QModelIndex idx;
    if( !view->isVisible() )
    {
        fillModel( window );
        // center on main window
        view->move( window->pos().x() + (window->width() - view->width()) / 2,
                    window->pos().y() + (window->height() - view->height()) / 2 );
        idx = model->index( 1, 0 );
        if( !idx.isValid() )
        {
            idx = model->index( 0, 0 );
        }
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
    view->selectionModel()->select( idx, QItemSelectionModel::Rows | QItemSelectionModel::ClearAndSelect );
    view->selectionModel()->setCurrentIndex( idx, QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows );
}

void DocumentSwitcherPlugin::fillModel( Sublime::MainWindow* window )
{
    model->clear();
    foreach( Sublime::View* v, documentLists[window][window->area()] )
    {
        QString txt = v->document()->title();
        KDevelop::IDocument* doc = dynamic_cast<KDevelop::IDocument*>( v->document() );
        if( doc ) {
            QString path = KDevelop::ICore::self()->projectController()->prettyFilePath( doc->url(), KDevelop::IProjectController::FormatPlain );
            // Remove trailing '/' as that looks ugly and creates empty string with lastIndexOf
            if( path.endsWith( '/' ) ) {
                path = path.left( path.length() - 1 );
            }
            // Relative path means we've got a project in front, so remove the 'inner' parts of the path to make it short
            // and display in a useful way
            if( QFileInfo( path ).isRelative() ) {
                path = path.left( path.indexOf( "/" ) + 1 ) + "..." + path.mid( path.lastIndexOf( '/' ) );
            } else {
                // Absolute, so try a very simple approach of using the first 6 and last 20 characters and elide the rest
                // On absolute paths the first letters won't be that useful, so make use of more of the suffix
                path = path.left( 6 ) + "..." + path.mid( path.length() - 20 );
            }
            txt = txt + " (" + path + ')';
        }
        model->appendRow( new QStandardItem( v->document()->icon(), txt ) );
    }
}

void DocumentSwitcherPlugin::walkBackward()
{
    Sublime::MainWindow* window = qobject_cast<Sublime::MainWindow*>( KDevelop::ICore::self()->uiController()->activeMainWindow() );
    if( !window || !documentLists.contains( window ) || !documentLists[window].contains( window->area() ) )
    {
        kWarning() << "This should not happen, tried to walk through document list of an unknown mainwindow!";
        return;
    }
    QModelIndex idx;
    if( !view->isVisible() )
    {
        fillModel( window );
        // center on mainwindow
        view->move( window->pos().x() + (window->width() - view->width()) / 2,
                    window->pos().y() + (window->height() - view->height()) / 2 );
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
    view->selectionModel()->select( idx, QItemSelectionModel::Rows | QItemSelectionModel::ClearAndSelect );
    view->selectionModel()->setCurrentIndex( idx, QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows );
}

DocumentSwitcherPlugin::~DocumentSwitcherPlugin()
{
}

void DocumentSwitcherPlugin::switchToView( const QModelIndex& idx )
{
    Q_UNUSED( idx );
    view->hide();
    if( view->selectionModel()->selectedRows().isEmpty() )
    {
        return;
    }
    int row = view->selectionModel()->selectedRows().first().row();
    
    Sublime::MainWindow* window = qobject_cast<Sublime::MainWindow*>( KDevelop::ICore::self()->uiController()->activeMainWindow() );
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
    foreach( QObject* mw, documentLists.keys() )
    {
        removeMainWindow( mw );
    }
    delete forwardAction;
    delete backwardAction;
    view->deleteLater();
}


void DocumentSwitcherPlugin::storeAreaViewList( Sublime::MainWindow* mainwindow, Sublime::Area* area )
{
    if( !documentLists.contains( mainwindow ) || !documentLists[mainwindow].contains(area) ) 
    {
        QMap<Sublime::Area*, QList<Sublime::View*> > areas;
        kDebug() << "adding area views for area:" << area << area->title() << "mainwindow:" << mainwindow << mainwindow->windowTitle();
        foreach( Sublime::View* v, area->views() ) {
            kDebug() << "view:" << v  << v->document()->title();
        }
        kDebug() << "done";
        areas.insert( area, area->views() );
        documentLists.insert( mainwindow, areas );
    }
    
}

void DocumentSwitcherPlugin::addMainWindow( Sublime::MainWindow* mainwindow ) 
{
    if( !mainwindow )  {
        return;
    }
    kDebug() << "adding mainwindow:" << mainwindow << mainwindow->windowTitle();
    kDebug() << "storing all views from area:" << mainwindow->area()->title() << mainwindow->area();
    storeAreaViewList( mainwindow, mainwindow->area() );
    kDebug() << "connecting signals on mainwindow";
    connect( mainwindow, SIGNAL(areaChanged(Sublime::Area*)), SLOT(changeArea(Sublime::Area*)) );
    connect( mainwindow, SIGNAL(activeViewChanged(Sublime::View*)), SLOT(changeView(Sublime::View*)) );
    connect( mainwindow, SIGNAL(viewAdded(Sublime::View*)), SLOT(addView(Sublime::View*)) );
    connect( mainwindow, SIGNAL(aboutToRemoveView(Sublime::View*)), SLOT(removeView(Sublime::View*)) );
    connect( mainwindow, SIGNAL(destroyed(QObject*)), SLOT(removeMainWindow(QObject*)));
    mainwindow->installEventFilter( this );
}

bool DocumentSwitcherPlugin::eventFilter( QObject* watched, QEvent* ev )
{
    Sublime::MainWindow* mw = dynamic_cast<Sublime::MainWindow*>( watched );
    if( mw && ev->type() == QEvent::WindowActivate )
    {
        enableActions(mw);
    }
    return QObject::eventFilter( watched, ev );
}

void DocumentSwitcherPlugin::addView( Sublime::View* view ) 
{
    Sublime::MainWindow* mainwindow = qobject_cast<Sublime::MainWindow*>( sender() );
    if( !mainwindow )
        return;

    kDebug() << "got signal from mainwindow:" << mainwindow << mainwindow->windowTitle();
    kDebug() << "its area is:" << mainwindow->area() << mainwindow->area()->title();
    kDebug() << "adding view:" << view << view->document()->title();
    enableActions( mainwindow );
    documentLists[mainwindow][mainwindow->area()].append( view );
}

void DocumentSwitcherPlugin::enableActions( Sublime::MainWindow* mw ) 
{
    forwardAction->setEnabled( documentLists[mw][mw->area()].size() > 1 );
    backwardAction->setEnabled( documentLists[mw][mw->area()].size() > 1 );
}


void DocumentSwitcherPlugin::removeMainWindow( QObject* obj ) 
{
    if( !obj || !documentLists.contains(obj) ) {
        return;
    }
    obj->removeEventFilter( this );
    disconnect( obj, 0, this, 0 );
    documentLists.remove( obj );
}


void DocumentSwitcherPlugin::changeArea( Sublime::Area* area ) 
{

    Sublime::MainWindow* mainwindow = qobject_cast<Sublime::MainWindow*>( sender() );
    Q_ASSERT( mainwindow );
    
    kDebug() << "area changed:" << area << area->title() << "mainwindow:" << mainwindow << mainwindow->windowTitle();

    //Since the main-window only emits aboutToRemoveView for views within the current area, we must forget all areas except the active one 
    documentLists.remove(mainwindow);
    
    if( !documentLists[mainwindow].contains( area ) )
    {
        kDebug() << "got area change, storing its views";
        storeAreaViewList( mainwindow, area );
    }
    enableActions( mainwindow );
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
    kDebug() << "moving view to front, list should now not contain this view anymore" << view << view->document()->title();
    kDebug() << "current area is:" << area << area->title() << "mainwnidow:" << mainwindow << mainwindow->windowTitle();;
    kDebug() << "idx of this view in list:" << documentLists[mainwindow][area].indexOf( view );
    documentLists[mainwindow][area].prepend( view );
    enableActions(mainwindow);
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

    kDebug() << "removing view, list should now not contain this view anymore" << view << view->document()->title();
    kDebug() << "current area is:" << area << area->title() << "mainwnidow:" << mainwindow << mainwindow->windowTitle();;
    kDebug() << "idx of this view in list:" << documentLists[mainwindow][area].indexOf( view );
    enableActions(mainwindow);
}

#include "documentswitcherplugin.moc"

