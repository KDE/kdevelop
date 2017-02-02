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

#include <QDBusConnection>
#include <QDomDocument>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMenu>
#include <QMenuBar>
#include <QMimeData>
#include <QUrl>

#include <KActionCollection>
#include <KLocalizedString>
#include <KShortcutsDialog>
#include <KTextEditor/Document>
#include <KTextEditor/View>
#include <KTextEditor/MainWindow>
#include <KWindowSystem>
#include <KXMLGUIFactory>
#include <kparts_version.h>

#include <sublime/area.h>
#include "shellextension.h"
#include "partcontroller.h"
#include "plugincontroller.h"
#include "projectcontroller.h"
#include "uicontroller.h"
#include "documentcontroller.h"
#include "debugcontroller.h"
#include "workingsetcontroller.h"
#include "sessioncontroller.h"
#include "sourceformattercontroller.h"
#include "areadisplay.h"
#include "project.h"
#include "debug.h"
#include "uiconfig.h"

#include <interfaces/isession.h>
#include <interfaces/iprojectcontroller.h>
#include <vcs/interfaces/ibasicversioncontrol.h>
#include <sublime/view.h>
#include <sublime/document.h>
#include <sublime/urldocument.h>
#include <sublime/container.h>
#include <util/path.h>
#include <util/widgetcolorizer.h>

using namespace KDevelop;

namespace {

QColor defaultColor(const QPalette& palette)
{
    return palette.foreground().color();
}

QColor colorForDocument(const QUrl& url, const QPalette& palette, const QColor& defaultColor)
{
    auto project = Core::self()->projectController()->findProjectForUrl(url);
    if (!project)
        return defaultColor;

    return WidgetColorizer::colorForId(qHash(project->path()), palette);
}

}

void MainWindow::applyMainWindowSettings(const KConfigGroup& config)
{
    if(!d->changingActiveView())
        KXmlGuiWindow::applyMainWindowSettings(config);
}

void MainWindow::createGUI(KParts::Part* part)
{
    //TODO remove if-clause once KF5 >= 5.24 is required
#if KPARTS_VERSION_MINOR >= 24
    Sublime::MainWindow::setWindowTitleHandling(false);
    Sublime::MainWindow::createGUI(part);
#else
    Sublime::MainWindow::createGUI(part);
    if (part) {
        // Don't let the Part control the main window caption -- we take care of that
        disconnect(part, SIGNAL(setWindowCaption(QString)),
                   this, SLOT(setCaption(QString)));
    }
#endif
}

void MainWindow::initializeCorners()
{
    const KConfigGroup cg = KSharedConfig::openConfig()->group( "UiSettings" );
    const int bottomleft = cg.readEntry( "BottomLeftCornerOwner", 0 );
    const int bottomright = cg.readEntry( "BottomRightCornerOwner", 0 );
    qCDebug(SHELL) << "Bottom Left:" << bottomleft;
    qCDebug(SHELL) << "Bottom Right:" << bottomright;

    // 0 means vertical dock (left, right), 1 means horizontal dock( top, bottom )
    if( bottomleft == 0 )
        setCorner( Qt::BottomLeftCorner, Qt::LeftDockWidgetArea );
    else if( bottomleft == 1 )
        setCorner( Qt::BottomLeftCorner, Qt::BottomDockWidgetArea );

    if( bottomright == 0 )
        setCorner( Qt::BottomRightCorner, Qt::RightDockWidgetArea );
    else if( bottomright == 1 )
        setCorner( Qt::BottomRightCorner, Qt::BottomDockWidgetArea );
}

MainWindow::MainWindow( Sublime::Controller *parent, Qt::WindowFlags flags )
        : Sublime::MainWindow( parent, flags )
{
    QDBusConnection::sessionBus().registerObject( QStringLiteral("/kdevelop/MainWindow"),
        this, QDBusConnection::ExportScriptableSlots );

    setAcceptDrops( true );
    initializeCorners();

    setObjectName( QStringLiteral("MainWindow") );
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
    if (memberList().count() == 1) {
        // We're closing down...
        Core::self()->shutdown();
    }

    delete d;
}

KTextEditorIntegration::MainWindow *MainWindow::kateWrapper() const
{
    return d->kateWrapper();
}

void MainWindow::split(Qt::Orientation orientation)
{
    d->split(orientation);
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
    QAction* before = nullptr;
    if (index > 0 && index < parent->actions().count())
        before = parent->actions().at(index);

    //KDevelop needs to ensure that separators defined as <Separator style="visible" />
    //are always shown in the menubar. For those, we create special disabled actions
    //instead of calling QMenuBar::addSeparator() because menubar separators are ignored
    if (element.tagName().toLower() == QLatin1String("separator")
            && element.attribute(QStringLiteral("style")) == QLatin1String("visible")) {
        if ( QMenuBar* bar = qobject_cast<QMenuBar*>( parent ) ) {
            QAction *separatorAction = new QAction(QStringLiteral("|"), this);
            bar->insertAction( before, separatorAction );
            separatorAction->setDisabled(true);
            return separatorAction;
        }
    }

    return KXMLGUIBuilder::createCustomElement(parent, index, element);
}

void MainWindow::dragEnterEvent( QDragEnterEvent* ev )
{
    const QMimeData* mimeData = ev->mimeData();
    if (mimeData->hasUrls()) {
        ev->acceptProposedAction();
    } else if (mimeData->hasText()) {
        // also take text which contains a URL
        const QUrl url = QUrl::fromUserInput(mimeData->text());
        if (url.isValid()) {
            ev->acceptProposedAction();
        }
    }
}

void MainWindow::dropEvent( QDropEvent* ev )
{
    Sublime::View* dropToView = viewForPosition(mapToGlobal(ev->pos()));
    if(dropToView)
        activateView(dropToView);

    QList<QUrl> urls;

    const QMimeData* mimeData = ev->mimeData();
    if (mimeData->hasUrls()) {
        urls = mimeData->urls();
    } else if (mimeData->hasText()) {
        const QUrl url = QUrl::fromUserInput(mimeData->text());
        if (url.isValid()) {
            urls << url;
        }
    }

    bool eventUsed = false;
    if (urls.size() == 1) {
        const QUrl& url = urls.at(0);
        // TODO: query also projectprovider plugins, and that before plain vcs plugins
        // e.g. KDE provider plugin could catch URLs from mirror or pickup kde:repo things

        auto* pluginController = Core::self()->pluginController();
        const auto& plugins = pluginController->allPluginsForExtension(QStringLiteral("org.kdevelop.IBasicVersionControl"));

        for (auto* plugin : plugins) {
            auto* iface = plugin->extension<IBasicVersionControl>();
            if (iface->isValidRemoteRepositoryUrl(url)) {
                Core::self()->projectControllerInternal()->fetchProjectFromUrl(url, plugin);
                eventUsed = true;
                break;
            }
        }
    }

    if (!eventUsed) {
        for(const auto& url : urls) {
            Core::self()->documentController()->openDocument(url);
        }
    }

    ev->acceptProposedAction();
}

void MainWindow::loadSettings()
{
    qCDebug(SHELL) << "Loading Settings";
    initializeCorners();

    updateAllTabColors();

    Sublime::MainWindow::loadSettings();
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

    connect(&dlg, &KShortcutsDialog::saved, this, &MainWindow::shortcutsChanged);
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
                qCDebug(SHELL) << "transferring setting shortcut for" << action->objectName();
                if(shortcuts.contains(action->objectName())) {
                    action->setShortcut(shortcuts[action->objectName()]);
                }
            }
        }
    }

}

void MainWindow::shortcutsChanged()
{
    KTextEditor::View *activeClient = Core::self()->documentController()->activeTextDocumentView();
    if(!activeClient)
        return;

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
    qCDebug(SHELL) << "Adding plugin-added connection";

    connect( Core::self()->pluginController(), &IPluginController::pluginLoaded,
             d, &MainWindowPrivate::addPlugin);
    connect( Core::self()->pluginController(), &IPluginController::pluginUnloaded,
             d, &MainWindowPrivate::removePlugin);
    connect( Core::self()->partController(), &IPartController::activePartChanged,
        d, &MainWindowPrivate::activePartChanged);
    connect( this, &MainWindow::activeViewChanged,
        d, &MainWindowPrivate::changeActiveView);

    foreach(IPlugin* plugin, Core::self()->pluginController()->loadedPlugins())
        d->addPlugin(plugin);

    guiFactory()->addClient(Core::self()->sessionController());
    guiFactory()->addClient(Core::self()->sourceFormatterControllerInternal());

    // Needed to re-plug the actions from the sessioncontroller as xmlguiclients don't
    // seem to remember which actions where plugged in.
    Core::self()->sessionController()->updateXmlGuiActionList();

    d->setupGui();

    //Queued so we process it with some delay, to make sure the rest of the UI has already adapted
    connect(Core::self()->documentController(), &IDocumentController::documentActivated, this, &MainWindow::updateCaption, Qt::QueuedConnection);
    connect(Core::self()->documentController(), &IDocumentController::documentActivated, this, &MainWindow::updateActiveDocumentConnection, Qt::QueuedConnection);
    connect(Core::self()->documentController(), &IDocumentController::documentClosed, this, &MainWindow::updateCaption, Qt::QueuedConnection);
    connect(Core::self()->documentController(), &IDocumentController::documentUrlChanged, this, &MainWindow::updateCaption, Qt::QueuedConnection);
    connect(Core::self()->sessionController()->activeSession(), &ISession::sessionUpdated, this, &MainWindow::updateCaption);

    connect(Core::self()->documentController(), &IDocumentController::documentOpened, this, &MainWindow::updateTabColor);
    connect(Core::self()->documentController(), &IDocumentController::documentUrlChanged, this, &MainWindow::updateTabColor);
    connect(this, &Sublime::MainWindow::viewAdded, this, &MainWindow::updateAllTabColors);
    connect(Core::self()->projectController(), &ProjectController::projectOpened, this, &MainWindow::updateAllTabColors, Qt::QueuedConnection);

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

void MainWindow::updateActiveDocumentConnection(IDocument* document)
{
    disconnect(d->activeDocumentReadWriteConnection);
    if (auto textDocument = document->textDocument()) {
        d->activeDocumentReadWriteConnection = connect(textDocument, &KTextEditor::Document::readWriteChanged,
                                                       this, &MainWindow::updateCaption);
    }
}

void MainWindow::updateCaption()
{
    const auto activeSession = Core::self()->sessionController()->activeSession();
    QString title = activeSession ? activeSession->description() : QString();

    if(area()->activeView())
    {
        if(!title.isEmpty())
            title += QLatin1String(" - [ ");

        Sublime::Document* doc = area()->activeView()->document();
        Sublime::UrlDocument* urlDoc = dynamic_cast<Sublime::UrlDocument*>(doc);
        if(urlDoc)
            title += Core::self()->projectController()->prettyFileName(urlDoc->url(), KDevelop::IProjectController::FormatPlain);
        else
            title += doc->title();

        auto activeDocument = Core::self()->documentController()->activeDocument();
        if (activeDocument && activeDocument->textDocument() && !activeDocument->textDocument()->isReadWrite())
            title += i18n(" (read only)");

        title += QLatin1String(" ]");
    }

    setCaption(title);
}

void MainWindow::updateAllTabColors()
{
    auto documentController = Core::self()->documentController();
    if (!documentController)
        return;

    const auto defaultColor = ::defaultColor(palette());
    if (UiConfig::colorizeByProject()) {
        QHash<const Sublime::View*, QColor> viewColors;
        foreach (auto container, containers()) {
            auto views = container->views();
            viewColors.reserve(views.size());
            viewColors.clear();
            foreach (auto view, views) {
                const auto urlDoc = qobject_cast<Sublime::UrlDocument*>(view->document());
                if (urlDoc) {
                    viewColors[view] = colorForDocument(urlDoc->url(), palette(), defaultColor);
                }
            }
            container->setTabColors(viewColors);
        }
    } else {
        foreach (auto container, containers()) {
            container->resetTabColors(defaultColor);
        }
    }
}

void MainWindow::updateTabColor(IDocument* doc)
{
    if (!UiConfig::self()->colorizeByProject())
        return;

    const auto color = colorForDocument(doc->url(), palette(), defaultColor(palette()));
    foreach (auto container, containers()) {
        foreach (auto view, container->views()) {
            const auto urlDoc = qobject_cast<Sublime::UrlDocument*>(view->document());
            if (urlDoc && urlDoc->url() == doc->url()) {
                container->setTabColor(view, color);
            }
        }
    }
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

void MainWindow::tabContextMenuRequested(Sublime::View* view, QMenu* menu)
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

void MainWindow::newTabRequested()
{
    Sublime::MainWindow::newTabRequested();

    d->fileNew();
}
