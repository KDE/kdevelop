/***************************************************************************
 *   Copyright 2009,2013 Andreas Pakulat <apaku@gmx.de>                    *
 *   Copyright 2013 Jarosław Sierant <jaroslaw.sierant@gmail.com>          *
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

#include <QApplication>
#include <QDir>
#include <QListView>
#include <QStandardItemModel>
#include <QScrollBar>

#include <KActionCollection>
#include <KLocalizedString>
#include <KPluginFactory>

#include <interfaces/icore.h>
#include <interfaces/iuicontroller.h>
#include <sublime/controller.h>
#include <sublime/view.h>
#include <sublime/area.h>
#include <sublime/mainwindow.h>
#include <sublime/document.h>

#include "documentswitchertreeview.h"
#include "debug.h"
#include <interfaces/iprojectcontroller.h>
#include <interfaces/idocument.h>
#include <interfaces/iproject.h>

#include <algorithm>

K_PLUGIN_FACTORY_WITH_JSON(DocumentSwitcherFactory, "kdevdocumentswitcher.json", registerPlugin<DocumentSwitcherPlugin>();)

//TODO: Show frame around view's widget while walking through
//TODO: Make the widget transparent

DocumentSwitcherPlugin::DocumentSwitcherPlugin(QObject *parent, const QVariantList &/*args*/)
    :KDevelop::IPlugin(QStringLiteral("kdevdocumentswitcher"), parent), view(nullptr)
{
    setXMLFile(QStringLiteral("kdevdocumentswitcher.rc"));
    qCDebug(PLUGIN_DOCUMENTSWITCHER) << "Adding active mainwindow from constructor" << KDevelop::ICore::self()->uiController()->activeMainWindow();
    addMainWindow( qobject_cast<Sublime::MainWindow*>( KDevelop::ICore::self()->uiController()->activeMainWindow() ) );
    connect( KDevelop::ICore::self()->uiController()->controller(), &Sublime::Controller::mainWindowAdded, this, &DocumentSwitcherPlugin::addMainWindow );

#ifdef Q_OS_MACOS
    // Qt/Mac swaps the Ctrl and Meta (Command) keys by default, so that shortcuts defined as Ctrl+X
    // become the platform-standard Command+X . Ideally we would map the document switcher shortcut to
    // Control+Tab (and thus Qt::META+Qt::Key_Tab) everywhere because Command+Tab and Command+Shift+Tab
    // are reserved system shortcuts that bring up the application switcher. The Control+Tab shortcut is 
    // inoperable on Mac, so we resort to the Alt (Option) key, unless the AA_MacDontSwapCtrlAndMeta
    // attribute is set.
    const Qt::Modifier shortcutAccelerator = QCoreApplication::testAttribute(Qt::AA_MacDontSwapCtrlAndMeta) ? Qt::CTRL : Qt::ALT;
#else
    const Qt::Modifier shortcutAccelerator = Qt::CTRL;
#endif

    forwardAction = actionCollection()->addAction ( QStringLiteral( "last_used_views_forward" ) );
    forwardAction->setText( i18n( "Last Used Views" ) );
    forwardAction->setIcon( QIcon::fromTheme( QStringLiteral( "go-next-view-page") ) );
    actionCollection()->setDefaultShortcut( forwardAction, shortcutAccelerator | Qt::Key_Tab );
    forwardAction->setWhatsThis( i18n( "Opens a list to walk through the list of last used views." ) );
    forwardAction->setStatusTip( i18n( "Walk through the list of last used views" ) );
    connect( forwardAction, &QAction::triggered, this, &DocumentSwitcherPlugin::walkForward );

    backwardAction = actionCollection()->addAction ( QStringLiteral( "last_used_views_backward" ) );
    backwardAction->setText( i18n( "Last Used Views (Reverse)" ) );
    backwardAction->setIcon( QIcon::fromTheme( QStringLiteral( "go-previous-view-page") ) );
    actionCollection()->setDefaultShortcut( backwardAction, shortcutAccelerator | Qt::SHIFT | Qt::Key_Tab );
    backwardAction->setWhatsThis( i18n( "Opens a list to walk through the list of last used views in reverse." ) );
    backwardAction->setStatusTip( i18n( "Walk through the list of last used views" ) );
    connect( backwardAction, &QAction::triggered, this, &DocumentSwitcherPlugin::walkBackward );

    view = new DocumentSwitcherTreeView( this );
    view->setSelectionBehavior( QAbstractItemView::SelectRows );
    view->setSelectionMode( QAbstractItemView::SingleSelection );
    view->setUniformRowHeights( true );
    view->setTextElideMode( Qt::ElideMiddle );
    view->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    view->addAction( forwardAction );
    view->addAction( backwardAction );
    view->setHeaderHidden( true );
    view->setIndentation( 10 );
    connect( view, &QListView::pressed, this, &DocumentSwitcherPlugin::switchToClicked );
    connect( view, &QListView::activated, this, &DocumentSwitcherPlugin::itemActivated );

    model = new QStandardItemModel( view );
    view->setModel( model );
}

void DocumentSwitcherPlugin::setViewGeometry(Sublime::MainWindow* window)
{
    const QSize centralSize = window->centralWidget()->size();

    // Maximum size of the view is 3/4th of the central widget (the editor area) so the view does not overlap the
    // mainwindow since that looks awkward.
    const QSize viewMaxSize( centralSize.width() * 3/4, centralSize.height() * 3/4 );

    // The actual view size should be as big as the columns/rows need it, but smaller than the max-size. This means
    // the view will get quite high with many open files but I think that is ok. Otherwise one can easily tweak the
    // max size to be only 1/2th of the central widget size
    const int rowHeight = view->sizeHintForRow(0);
    const int frameWidth = view->frameWidth();
    const QSize viewSize( std::min( view->sizeHintForColumn(0) + 2 * frameWidth + view->verticalScrollBar()->width(), viewMaxSize.width() ),
                          std::min( std::max( rowHeight * view->model()->rowCount() + 2 * frameWidth, rowHeight * 6 ), viewMaxSize.height() ) );

    // Position should be central over the editor area, so map to global from parent of central widget since
    // the view is positioned in global coords
    QPoint centralWidgetPos = window->mapToGlobal( window->centralWidget()->pos() );
    const int xPos = std::max(0, centralWidgetPos.x() + (centralSize.width()  - viewSize.width()  ) / 2);
    const int yPos = std::max(0, centralWidgetPos.y() + (centralSize.height() - viewSize.height() ) / 2);

    view->setFixedSize(viewSize);
    view->move(xPos, yPos);
}

void DocumentSwitcherPlugin::walk(const int from, const int to)
{
    Sublime::MainWindow* window = qobject_cast<Sublime::MainWindow*>( KDevelop::ICore::self()->uiController()->activeMainWindow() );
    if( !window || !documentLists.contains( window ) || !documentLists[window].contains( window->area() ) )
    {
        qCWarning(PLUGIN_DOCUMENTSWITCHER) << "This should not happen, tried to walk through document list of an unknown mainwindow!";
        return;
    }
    QModelIndex idx;
    const int step = from < to ? 1 : -1;
    if(!view->isVisible())
    {
        fillModel(window);
        setViewGeometry(window);
        idx = model->index(from + step, 0);
        if(!idx.isValid()) { idx = model->index(0, 0); }
        view->show();
    } else {
        int newRow = view->selectionModel()->currentIndex().row() + step;
        if(newRow == to + step) { newRow = from; }
        idx = model->index(newRow, 0);
    }
    view->selectionModel()->select(idx, QItemSelectionModel::Rows | QItemSelectionModel::ClearAndSelect);
    view->selectionModel()->setCurrentIndex(idx, QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows);
}


void DocumentSwitcherPlugin::walkForward() { walk(0, model->rowCount()-1); }

void DocumentSwitcherPlugin::walkBackward() { walk(model->rowCount()-1, 0); }

void DocumentSwitcherPlugin::fillModel( Sublime::MainWindow* window )
{
    model->clear();
    auto projectController = KDevelop::ICore::self()->projectController();
    foreach( Sublime::View* v, documentLists[window][window->area()] )
    {
        using namespace KDevelop;
        Sublime::Document const* const slDoc = v->document();
        if( !slDoc )
        {
            continue;
        }
        QString itemText = slDoc->title();// file name
        IDocument const* const doc = dynamic_cast<IDocument*>(v->document());
        IProject* project = nullptr;
        if( doc )
        {
            QString path = projectController->prettyFilePath(doc->url(),
                                                                              IProjectController::FormatPlain);
            const bool isPartOfOpenProject = QDir::isRelativePath(path);
            if( path.endsWith('/') )
            {
                path.remove(path.length() - 1, 1);
            }
            if( isPartOfOpenProject )
            {
                const int projectNameSize = path.indexOf(QLatin1Char(':'));

                // first: project name, second: path to file in project (might be just '/' when the file is in the project root dir)
                const QPair<QString, QString> fileInProjectInfo = (projectNameSize < 0)
                    ? qMakePair(path, QStringLiteral("/"))
                    : qMakePair(path.left(projectNameSize), path.mid(projectNameSize + 1));

                itemText = QStringLiteral("%1 (%2:%3)").arg(itemText,
                                                            fileInProjectInfo.first,
                                                            fileInProjectInfo.second);
            } else
            {
                itemText = itemText + " (" + path + ')';
            }
            project = projectController->findProjectForUrl(doc->url());
        }
        auto item = new QStandardItem( slDoc->icon(), itemText );
        item->setData(QVariant::fromValue(project), DocumentSwitcherTreeView::ProjectRole);
        model->appendRow( item );
    }
}

DocumentSwitcherPlugin::~DocumentSwitcherPlugin()
{
}

void DocumentSwitcherPlugin::switchToClicked( const QModelIndex& idx )
{
    view->selectionModel()->select(idx, QItemSelectionModel::ClearAndSelect);
    itemActivated(idx);
}

void DocumentSwitcherPlugin::itemActivated( const QModelIndex& idx )
{
    Q_UNUSED( idx );
    if( view->selectionModel()->selectedRows().isEmpty() )
    {
        return;
    }
    int row = view->selectionModel()->selectedRows().first().row();

    Sublime::MainWindow* window = qobject_cast<Sublime::MainWindow*>( KDevelop::ICore::self()->uiController()->activeMainWindow() );
    Sublime::View* activatedView = nullptr;
    if( window && documentLists.contains( window ) && documentLists[window].contains( window->area() ) )
    {
        const QList<Sublime::View*> l = documentLists[window][window->area()];
        if( row >= 0 && row < l.size() )
        {
            activatedView = l.at( row );
        }
    }
    if( activatedView ) {
        if( QApplication::mouseButtons() & Qt::MiddleButton )
        {
            window->area()->closeView( activatedView );
            fillModel( window );
            if ( model->rowCount() == 0 ) {
                view->hide();
            } else {
                view->selectionModel()->select( view->model()->index(0, 0), QItemSelectionModel::ClearAndSelect );
            }
        } else
        {
            window->activateView( activatedView );
            view->hide();
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
        QHash<Sublime::Area*, QList<Sublime::View*> > areas;
        qCDebug(PLUGIN_DOCUMENTSWITCHER) << "adding area views for area:" << area << area->title() << "mainwindow:" << mainwindow << mainwindow->windowTitle();
        foreach( Sublime::View* v, area->views() ) {
            qCDebug(PLUGIN_DOCUMENTSWITCHER) << "view:" << v  << v->document()->title();
        }
        qCDebug(PLUGIN_DOCUMENTSWITCHER) << "done";
        areas.insert( area, area->views() );
        documentLists.insert( mainwindow, areas );
    }

}

void DocumentSwitcherPlugin::addMainWindow( Sublime::MainWindow* mainwindow )
{
    if( !mainwindow )  {
        return;
    }
    qCDebug(PLUGIN_DOCUMENTSWITCHER) << "adding mainwindow:" << mainwindow << mainwindow->windowTitle();
    qCDebug(PLUGIN_DOCUMENTSWITCHER) << "storing all views from area:" << mainwindow->area()->title() << mainwindow->area();
    storeAreaViewList( mainwindow, mainwindow->area() );
    qCDebug(PLUGIN_DOCUMENTSWITCHER) << "connecting signals on mainwindow";
    connect( mainwindow, &Sublime::MainWindow::areaChanged, this, &DocumentSwitcherPlugin::changeArea );
    connect( mainwindow, &Sublime::MainWindow::activeViewChanged, this, &DocumentSwitcherPlugin::changeView );
    connect( mainwindow, &Sublime::MainWindow::viewAdded, this, &DocumentSwitcherPlugin::addView );
    connect( mainwindow, &Sublime::MainWindow::aboutToRemoveView, this, &DocumentSwitcherPlugin::removeView );
    connect( mainwindow, &Sublime::MainWindow::destroyed, this, &DocumentSwitcherPlugin::removeMainWindow);
    mainwindow->installEventFilter( this );
}

bool DocumentSwitcherPlugin::eventFilter( QObject* watched, QEvent* ev )
{
    Sublime::MainWindow* mw = dynamic_cast<Sublime::MainWindow*>( watched );
    if( mw && ev->type() == QEvent::WindowActivate )
    {
        enableActions();
    }
    return QObject::eventFilter( watched, ev );
}

void DocumentSwitcherPlugin::addView( Sublime::View* view )
{
    Sublime::MainWindow* mainwindow = qobject_cast<Sublime::MainWindow*>( sender() );
    if( !mainwindow )
        return;

    qCDebug(PLUGIN_DOCUMENTSWITCHER) << "got signal from mainwindow:" << mainwindow << mainwindow->windowTitle()
        << "its area is:" << mainwindow->area() << mainwindow->area()->title()
        << "adding view:" << view << view->document()->title();
    enableActions();
    documentLists[mainwindow][mainwindow->area()].append( view );
}

void DocumentSwitcherPlugin::enableActions()
{
    forwardAction->setEnabled(true);
    backwardAction->setEnabled(true);
}


void DocumentSwitcherPlugin::removeMainWindow( QObject* obj )
{
    if( !obj || !documentLists.contains(obj) ) {
        return;
    }
    obj->removeEventFilter( this );
    disconnect( obj, nullptr, this, nullptr );
    documentLists.remove( obj );
}


void DocumentSwitcherPlugin::changeArea( Sublime::Area* area )
{

    Sublime::MainWindow* mainwindow = qobject_cast<Sublime::MainWindow*>( sender() );
    Q_ASSERT( mainwindow );

    qCDebug(PLUGIN_DOCUMENTSWITCHER) << "area changed:" << area << area->title() << "mainwindow:" << mainwindow << mainwindow->windowTitle();

    //Since the main-window only emits aboutToRemoveView for views within the current area, we must forget all areas except the active one
    documentLists.remove(mainwindow);

    if( !documentLists[mainwindow].contains( area ) )
    {
        qCDebug(PLUGIN_DOCUMENTSWITCHER) << "got area change, storing its views";
        storeAreaViewList( mainwindow, area );
    }
    enableActions();
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
    qCDebug(PLUGIN_DOCUMENTSWITCHER) << "moving view to front, list should now not contain this view anymore" << view << view->document()->title();
    qCDebug(PLUGIN_DOCUMENTSWITCHER) << "current area is:" << area << area->title() << "mainwnidow:" << mainwindow << mainwindow->windowTitle();;
    qCDebug(PLUGIN_DOCUMENTSWITCHER) << "idx of this view in list:" << documentLists[mainwindow][area].indexOf( view );
    documentLists[mainwindow][area].prepend( view );
    enableActions();
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

    qCDebug(PLUGIN_DOCUMENTSWITCHER) << "removing view, list should now not contain this view anymore" << view << view->document()->title();
    qCDebug(PLUGIN_DOCUMENTSWITCHER) << "current area is:" << area << area->title() << "mainwnidow:" << mainwindow << mainwindow->windowTitle();;
    qCDebug(PLUGIN_DOCUMENTSWITCHER) << "idx of this view in list:" << documentLists[mainwindow][area].indexOf( view );
    enableActions();
}

#include "documentswitcherplugin.moc"

