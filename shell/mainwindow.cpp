/* This file is part of the KDevelop project
Copyright 2002 Falk Brettschneider <falkbr@kdevelop.org>
Copyright 2003 John Firebaugh <jfirebaugh@kde.org>
Copyright 2006 Adam Treat <treat@kde.org>
Copyright 2006, 2007 Alexander Dymo <adymo@kdevelop.org>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public License
along with this library; see the file COPYING.LIB.  If not, write to
the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
Boston, MA 02110-1301, USA.
*/
#include "mainwindow.h"
#include "mainwindow_p.h"

#include <QtCore/QHash>
#include <QDockWidget>
#include <QDragEnterEvent>
#include <QDomDocument>
#include <QDropEvent>
#include <QMenuBar>
#include <QtDBus/QDBusConnection>

#include <KDE/KApplication>
#include <KDE/KActionCollection>
#include <kmenu.h>
#include <kglobal.h>
#include <klocale.h>
#include <ktemporaryfile.h>
#include <kactioncollection.h>
#include <kdebug.h>
#include <ktexteditor/document.h>
#include <ktexteditor/view.h>
#include <kshortcutsdialog.h>
#include <kxmlguifactory.h>
#include <ktoggleaction.h>
#include <KWindowSystem>
#include <KMenuBar>

#include <sublime/area.h>
#include "shellextension.h"
#include "partcontroller.h"
#include "plugincontroller.h"
#include "uicontroller.h"
#include "documentcontroller.h"
#include "debugcontroller.h"
#include "workingsetcontroller.h"
#include "sessioncontroller.h"
#include "sourceformattercontroller.h"
#include "areadisplay.h"

#include <interfaces/isession.h>
#include <interfaces/iprojectcontroller.h>
#include <sublime/view.h>
#include <sublime/document.h>
#include <sublime/urldocument.h>

namespace KDevelop
{

void MainWindow::applyMainWindowSettings(const KConfigGroup& config, bool force)
{
    if(!d->changingActiveView())
        KXmlGuiWindow::applyMainWindowSettings(config, force);
}

MainWindow::MainWindow( Sublime::Controller *parent, Qt::WFlags flags )
        : Sublime::MainWindow( parent, flags )
{
    QDBusConnection::sessionBus().registerObject( "/kdevelop/MainWindow",
        this, QDBusConnection::ExportScriptableSlots );

    setAcceptDrops( true );
    KConfigGroup cg = KGlobal::config()->group( "UiSettings" );
    int bottomleft = cg.readEntry( "BottomLeftCornerOwner", 0 );
    int bottomright = cg.readEntry( "BottomRightCornerOwner", 0 );
    kDebug() << "Bottom Left:" << bottomleft;
    kDebug() << "Bottom Right:" << bottomright;

    // 0 means vertical dock (left, right), 1 means horizontal dock( top, bottom )
    if( bottomleft == 0 )
        setCorner( Qt::BottomLeftCorner, Qt::LeftDockWidgetArea );
    else if( bottomleft == 1 )
        setCorner( Qt::BottomLeftCorner, Qt::BottomDockWidgetArea );

    if( bottomright == 0 )
        setCorner( Qt::BottomRightCorner, Qt::RightDockWidgetArea );
    else if( bottomright == 1 )
        setCorner( Qt::BottomRightCorner, Qt::BottomDockWidgetArea );

    setObjectName( "MainWindow" );
    d = new MainWindowPrivate(this);

    setStandardToolBarMenuEnabled( true );
    d->setupActions();

    if( !ShellExtension::getInstance()->xmlFile().isEmpty() )
    {
        setXMLFile( ShellExtension::getInstance() ->xmlFile() );
    }

    menuBar()->setCornerWidget(new AreaDisplay(this), Qt::TopRightCorner);
}

MainWindow::~ MainWindow()
{
    Core::self()->uiControllerInternal()->mainWindowDeleted(this);

    if (memberList().count() == 1) {
        // We're closing down...
        Core::self()->shutdown();
    }

    delete d;
}

void MainWindow::ensureVisible()
{
    if (isMinimized()) {
        if (isMaximized()) {
            showMaximized();
        } else {
            showNormal();
        }
    }
    KWindowSystem::forceActiveWindow(winId());
}

QAction* MainWindow::createCustomElement(QWidget* parent, int index, const QDomElement& element)
{
    QAction* before = 0L;
    if (index > 0 && index < parent->actions().count())
        before = parent->actions().at(index);

    //KDevelop needs to ensure that separators defined as <Separator style="visible" />
    //are always shown in the menubar. For those, we create special disabled actions
    //instead of calling QMenuBar::addSeparator() because menubar separators are ignored
    if (element.tagName().toLower() == QLatin1String("separator")
            && element.attribute("style") == QLatin1String("visible")) {
        if ( QMenuBar* bar = qobject_cast<QMenuBar*>( parent ) ) {
            QAction *separatorAction = new QAction("|", this);
            bar->insertAction( before, separatorAction );
            separatorAction->setDisabled(true);
            return separatorAction;
        }
    }

    return KXMLGUIBuilder::createCustomElement(parent, index, element);
}

void MainWindow::dragEnterEvent( QDragEnterEvent* ev )
{
    if( ev->mimeData()->hasFormat( "text/uri-list" ) && ev->mimeData()->hasUrls() )
    {
        ev->acceptProposedAction();
    }
}

void MainWindow::dropEvent( QDropEvent* ev )
{
    Sublime::View* dropToView = viewForPosition(mapToGlobal(ev->pos()));
    if(dropToView)
        activateView(dropToView);
    
    foreach( const QUrl& u, ev->mimeData()->urls() )
    {
        Core::self()->documentController()->openDocument( KUrl( u ) );
    }
    ev->acceptProposedAction();
}

void MainWindow::loadSettings()
{
    kDebug() << "Loading Settings";
    KConfigGroup cg = KGlobal::config()->group( "UiSettings" );

    // dock widget corner layout
    int bottomleft = cg.readEntry( "BottomLeftCornerOwner", 0 );
    int bottomright = cg.readEntry( "BottomRightCornerOwner", 0 );
    kDebug() << "Bottom Left:" << bottomleft;
    kDebug() << "Bottom Right:" << bottomright;

    // 0 means vertical dock (left, right), 1 means horizontal dock( top, bottom )
    if( bottomleft == 0 )
        setCorner( Qt::BottomLeftCorner, Qt::LeftDockWidgetArea );
    else if( bottomleft == 1 )
        setCorner( Qt::BottomLeftCorner, Qt::BottomDockWidgetArea );

    if( bottomright == 0 )
        setCorner( Qt::BottomRightCorner, Qt::RightDockWidgetArea );
    else if( bottomright == 1 )
        setCorner( Qt::BottomRightCorner, Qt::BottomDockWidgetArea );

    Sublime::MainWindow::loadSettings();
}

void MainWindow::saveSettings()
{
    Sublime::MainWindow::saveSettings();
}


void MainWindow::configureShortcuts()
{
    ///Workaround for a problem with the actions: Always start the shortcut-configuration in the first mainwindow, then propagate the updated
    ///settings into the other windows


// We need to bring up the shortcut dialog ourself instead of 
//      Core::self()->uiControllerInternal()->mainWindows()[0]->guiFactory()->configureShortcuts();
// so we can connect to the saved() signal to propagate changes in the editor shortcuts

   KShortcutsDialog dlg(KShortcutsEditor::AllActions, KShortcutsEditor::LetterShortcutsAllowed, this);
    foreach (KXMLGUIClient *client, Core::self()->uiControllerInternal()->mainWindows()[0]->guiFactory()->clients())
    {
        if(client && !client->xmlFile().isEmpty())
            dlg.addCollection( client->actionCollection() );
    }
    
    connect(&dlg, SIGNAL(saved()), SLOT(shortcutsChanged()));
    dlg.configure(true);

    QMap<QString, QKeySequence> shortcuts;
    foreach(KXMLGUIClient* client, Core::self()->uiControllerInternal()->mainWindows()[0]->guiFactory()->clients()) {
        foreach(QAction* action, client->actionCollection()->actions()) {
            if(!action->objectName().isEmpty()) {
                shortcuts[action->objectName()] = action->shortcut();
            }
        }
    }
    
    for(int a = 1; a < Core::self()->uiControllerInternal()->mainWindows().size(); ++a) {
        foreach(KXMLGUIClient* client, Core::self()->uiControllerInternal()->mainWindows()[a]->guiFactory()->clients()) {
            foreach(QAction* action, client->actionCollection()->actions()) {
                kDebug() << "transferring setting shortcut for" << action->objectName();
                if(shortcuts.contains(action->objectName())) {
                    action->setShortcut(shortcuts[action->objectName()]);
                }
            }
        }
    }
    
}

void MainWindow::shortcutsChanged()
{
    //propagate shortcut changes to all the opened text documents by reloading the UI XML file
    IDocument* activeDoc = Core::self()->documentController()->activeDocument();
    if (!activeDoc || !activeDoc->textDocument()) {
        return;
    }

    KTextEditor::View *activeClient = activeDoc->textDocument()->activeView();

    foreach(IDocument * doc, Core::self()->documentController()->openDocuments()) {
        KTextEditor::Document *textDocument = doc->textDocument();
        if (textDocument) {
            foreach(KTextEditor::View *client, textDocument->views()) {
                if (client != activeClient) {
                    client->reloadXML();
                }
            }
        }
    }
}


void MainWindow::initialize()
{
    KStandardAction::keyBindings(this, SLOT(configureShortcuts()), actionCollection());
    setupGUI( KXmlGuiWindow::ToolBar | KXmlGuiWindow::Create | KXmlGuiWindow::Save );
    
    Core::self()->partController()->addManagedTopLevelWidget(this);
    kDebug() << "Adding plugin-added connection";
    
    connect( Core::self()->pluginController(), SIGNAL(pluginLoaded(KDevelop::IPlugin*)),
             d, SLOT(addPlugin(KDevelop::IPlugin*)));
    connect( Core::self()->pluginController(), SIGNAL(pluginUnloaded(KDevelop::IPlugin*)),
             d, SLOT(removePlugin(KDevelop::IPlugin*)));
    connect( Core::self()->partController(), SIGNAL(activePartChanged(KParts::Part*)),
        d, SLOT(activePartChanged(KParts::Part*)));
    connect( this, SIGNAL(activeViewChanged(Sublime::View*)),
        d, SLOT(changeActiveView(Sublime::View*)));
    
    foreach(IPlugin* plugin, Core::self()->pluginController()->loadedPlugins())
        d->addPlugin(plugin);
    
    guiFactory()->addClient(Core::self()->sessionController());
    guiFactory()->addClient(Core::self()->sourceFormatterControllerInternal());
    // Needed to re-plug the actions from the sessioncontroller as xmlguiclients don't
    // seem to remember which actions where plugged in.
    Core::self()->sessionController()->plugActions();
    d->setupGui();
    
    //Queued so we process it with some delay, to make sure the rest of the UI has already adapted
    connect(Core::self()->documentController(), SIGNAL(documentActivated(KDevelop::IDocument*)), SLOT(updateCaption()), Qt::QueuedConnection);
    connect(Core::self()->documentController(), SIGNAL(documentClosed(KDevelop::IDocument*)), SLOT(updateCaption()), Qt::QueuedConnection);
    connect(Core::self()->documentController(), SIGNAL(documentUrlChanged(KDevelop::IDocument*)), SLOT(updateCaption()), Qt::QueuedConnection);

    connect(Core::self()->sessionController()->activeSession(), SIGNAL(sessionUpdated(KDevelop::ISession*)), SLOT(updateCaption()));

    updateCaption();
}

void MainWindow::cleanup()
{
}

void MainWindow::setVisible( bool visible )
{
    KXmlGuiWindow::setVisible( visible );
    emit finishedLoading();
}

bool MainWindow::queryClose()
{
    if (!Core::self()->documentControllerInternal()->saveAllDocumentsForWindow(this, IDocument::Default))
        return false;

    return Sublime::MainWindow::queryClose();
}

void MainWindow::updateCaption()
{
    QString title = Core::self()->sessionController()->activeSession()->description();
    
    if(area()->activeView())
    {
        if(!title.isEmpty())
            title += " - [ ";
        
        Sublime::Document* doc = area()->activeView()->document();
        Sublime::UrlDocument* urlDoc = dynamic_cast<Sublime::UrlDocument*>(doc);
        if(urlDoc)
            title += Core::self()->projectController()->prettyFileName(KUrl(urlDoc->documentSpecifier()), KDevelop::IProjectController::FormatPlain);
        else
            title += doc->title();
        
        title += " ]";
    }
    
    setCaption(title);
}

void MainWindow::registerStatus(QObject* status)
{
    d->registerStatus(status);
}

void MainWindow::initializeStatusBar()
{
    d->setupStatusBar();
}

void MainWindow::showErrorMessage(const QString& message, int timeout)
{
    d->showErrorMessage(message, timeout);
}

void MainWindow::tabContextMenuRequested(Sublime::View* view, KMenu* menu)
{
    Sublime::MainWindow::tabContextMenuRequested(view, menu);
    d->tabContextMenuRequested(view, menu);
}

void MainWindow::tabToolTipRequested(Sublime::View* view, Sublime::Container* container, int tab)
{
    d->tabToolTipRequested(view, container, tab);
}

void MainWindow::dockBarContextMenuRequested(Qt::DockWidgetArea area, const QPoint& position)
{
    d->dockBarContextMenuRequested(area, position);
}

}

#include "mainwindow.moc"
