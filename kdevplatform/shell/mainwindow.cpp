/*
    SPDX-FileCopyrightText: 2002 Falk Brettschneider <falkbr@kdevelop.org>
    SPDX-FileCopyrightText: 2003 John Firebaugh <jfirebaugh@kde.org>
    SPDX-FileCopyrightText: 2006 Adam Treat <treat@kde.org>
    SPDX-FileCopyrightText: 2006, 2007 Alexander Dymo <adymo@kdevelop.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "mainwindow.h"
#include "mainwindow_p.h"

#include <QDBusConnection>
#include <QDomElement>
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
#include <KWindowSystem>
#include <KXMLGUIFactory>

#define HAVE_X11 __has_include(<KX11Extras>)
#if HAVE_X11
#include <KX11Extras>
#endif

#include <sublime/area.h>
#include "shellextension.h"
#include "partcontroller.h"
#include "plugincontroller.h"
#include "projectcontroller.h"
#include "uicontroller.h"
#include "documentcontroller.h"
#include "workingsetcontroller.h"
#include "sessioncontroller.h"
#include "sourceformattercontroller.h"
#include "areadisplay.h"
#include "project.h"
#include "debug.h"
#include "uiconfig.h"
#include "ktexteditorpluginintegration.h"

#include <interfaces/isession.h>
#include <interfaces/iprojectcontroller.h>
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
    return palette.windowText().color();
}

QColor colorForDocument(const QUrl& url, const QPalette& palette, const QColor& defaultColor)
{
    auto project = Core::self()->projectController()->findProjectForUrl(url);
    if (!project)
        return defaultColor;

    return WidgetColorizer::colorForId(qHash(project->path()), palette);
}

}

void MainWindow::createGUI(KParts::Part* part)
{
    Sublime::MainWindow::setWindowTitleHandling(false);
    Sublime::MainWindow::createGUI(part);
}

void MainWindow::loadCornerSettings()
{
    const KConfigGroup cg = KSharedConfig::openConfig()->group(QStringLiteral("UiSettings"));
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

    setObjectName( QStringLiteral("MainWindow") );
    d_ptr = new MainWindowPrivate(this);

    Q_D(MainWindow);

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

    delete d_ptr;
}

KTextEditorIntegration::MainWindow *MainWindow::kateWrapper() const
{
    Q_D(const MainWindow);

    return d->kateWrapper();
}

void MainWindow::split(Qt::Orientation orientation)
{
    Q_D(MainWindow);

    d->split(orientation);
}

void MainWindow::loadUiPreferences()
{
    loadCornerSettings();
    updateAllTabColors();
    Sublime::MainWindow::loadUiPreferences();
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

#if HAVE_X11
    if (KWindowSystem::isPlatformX11()) {
        KX11Extras::forceActiveWindow(winId());
        return;
    }
#endif
    activateWindow();
}

QAction* MainWindow::createCustomElement(QWidget* parent, int index, const QDomElement& element)
{
    QAction* before = nullptr;
    if (index > 0 && index < parent->actions().count())
        before = parent->actions().at(index);

    //KDevelop needs to ensure that separators defined as <Separator style="visible" />
    //are always shown in the menubar. For those, we create special disabled actions
    //instead of calling QMenuBar::addSeparator() because menubar separators are ignored
    if (element.tagName().compare(QLatin1String("separator"), Qt::CaseInsensitive) == 0
            && element.attribute(QStringLiteral("style")) == QLatin1String("visible")) {
        if ( auto* bar = qobject_cast<QMenuBar*>( parent ) ) {
            auto* separatorAction = new QAction(QStringLiteral("|"), this);
            bar->insertAction( before, separatorAction );
            separatorAction->setDisabled(true);
            return separatorAction;
        }
    }

    return KXMLGUIBuilder::createCustomElement(parent, index, element);
}

bool KDevelop::MainWindow::event( QEvent* ev )
{
    if ( ev->type() == QEvent::PaletteChange )
        updateAllTabColors();
    return Sublime::MainWindow::event(ev);
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
    Sublime::View* dropToView = viewForPosition(mapToGlobal(ev->position().toPoint()));
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
        eventUsed = Core::self()->projectControllerInternal()->fetchProjectFromUrl(urls.at(0), ProjectController::NoFetchFlags);
    }

    if (!eventUsed) {
        for (const auto& url : std::as_const(urls)) {
            Core::self()->documentController()->openDocument(url);
        }
    }

    ev->acceptProposedAction();
}

void MainWindow::loadSettings()
{
    Q_D(const MainWindow);

    if (!d->isGuiSetUp()) {
        // UiController() invokes this->loadSettings() before UiController::initialize() invokes this->initialize().
        // this->initialize() invokes KXmlGuiWindow::setupGUI(), which resizes the window to its default size.
        // Sublime::MainWindow::loadSettings() calls KXmlGuiWindow::applyMainWindowSettings(), which applies the size
        // stored in the main window config *once*. If the size stored in config is applied before the default resizing,
        // the window always ends up having the default size. Prevent this by returning early if the GUI is not set up
        // (not initialized) yet. The settings are still always loaded when UiController::loadAllAreas() is invoked
        // after this->initialize().
        return;
    }

    qCDebug(SHELL) << "Loading Settings";

    Sublime::MainWindow::loadSettings();
    // Sublime::MainWindow::loadSettings() invokes QMainWindow::restoreState(), which restores corner
    // settings. Override the corner settings to make sure they match KDevelop's custom corner options.
    loadCornerSettings();
}

void MainWindow::configureShortcuts()
{
    ///Workaround for a problem with the actions: Always start the shortcut-configuration in the first mainwindow, then propagate the updated
    ///settings into the other windows


// We need to bring up the shortcut dialog ourself instead of
//      Core::self()->uiControllerInternal()->mainWindows()[0]->guiFactory()->configureShortcuts();
// so we can connect to the saved() signal to propagate changes in the editor shortcuts

   KShortcutsDialog dlg(KShortcutsEditor::AllActions, KShortcutsEditor::LetterShortcutsAllowed, this);

    const auto firstMainWindowClientsBefore = Core::self()->uiControllerInternal()->mainWindows()[0]->guiFactory()->clients();
    for (KXMLGUIClient* client : firstMainWindowClientsBefore) {
        if(client && !client->xmlFile().isEmpty())
            dlg.addCollection( client->actionCollection() );
    }

    connect(&dlg, &KShortcutsDialog::saved, this, &MainWindow::shortcutsChanged);
    dlg.configure(true);

    QMap<QString, QKeySequence> shortcuts;
    // querying again just in case something changed behind our back
    const auto firstMainWindowClientsAfter = Core::self()->uiControllerInternal()->mainWindows()[0]->guiFactory()->clients();
    for (KXMLGUIClient* client : firstMainWindowClientsAfter) {
        const auto actions = client->actionCollection()->actions();
        for (QAction* action : actions) {
            if(!action->objectName().isEmpty()) {
                shortcuts[action->objectName()] = action->shortcut();
            }
        }
    }

    for(int a = 1; a < Core::self()->uiControllerInternal()->mainWindows().size(); ++a) {
        const auto clients = Core::self()->uiControllerInternal()->mainWindows()[a]->guiFactory()->clients();
        for (KXMLGUIClient* client : clients) {
            const auto actions = client->actionCollection()->actions();
            for (QAction* action : actions) {
                qCDebug(SHELL) << "transferring setting shortcut for" << action->objectName();
                const auto shortcutIt = shortcuts.constFind(action->objectName());
                if (shortcutIt != shortcuts.constEnd()) {
                    action->setShortcut(*shortcutIt);
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

    const auto documents = Core::self()->documentController()->openDocuments();
    for (IDocument* doc : documents) {
        KTextEditor::Document *textDocument = doc->textDocument();
        if (textDocument) {
            const auto views = textDocument->views();
            for (KTextEditor::View* client : views) {
                if (client != activeClient) {
                    client->reloadXML();
                }
            }
        }
    }
}


void MainWindow::initialize()
{
    Q_D(MainWindow);

    KStandardAction::keyBindings(this, SLOT(configureShortcuts()), actionCollection());

    // Do not pass the Save option to setupGUI(), because main window settings are loaded from and saved to
    // different config groups depending on the current area. So KMainWindow::autoSaveGroup() would have to
    // be changed accordingly each time another area is switched to. Also Sublime::MainWindow::saveSettings()
    // and Sublime::MainWindow::loadSettings() do more than just call saveMainWindowSettings() and
    // applyMainWindowSettings() respectively. applyMainWindowSettings() is virtual and can be overridden, but
    // there is no way to customize behavior of the non-virtual function KMainWindow::saveMainWindowSettings().
    // Auto-saving also requires recovering from a possible last auto-save while in Concentration Mode - perhaps
    // force-exit Concentration Mode even if it is off at the end of Sublime::MainWindow::loadSettings().
    // On KDevelop exit, UiController::cleanup() calls Sublime::MainWindow::saveSettings() for each main window.
    // Therefore, auto-saving main window settings is useful only in case KDevelop crashes. Auto-saving correctly
    // might be possible, but perhaps not worth the likely significant implementation complexity increase.
    setupGUI(QSize{870, 650}, ToolBar);
    createGUI(nullptr);

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
    connect(Core::self()->sourceFormatterControllerInternal(), &SourceFormatterController::hasFormattersChanged,
             d, &MainWindowPrivate::updateSourceFormatterGuiClient);

    const auto plugins = Core::self()->pluginController()->loadedPlugins();
    for (IPlugin* plugin : plugins) {
        d->addPlugin(plugin);
    }

    guiFactory()->addClient(Core::self()->sessionController());
    if (Core::self()->sourceFormatterControllerInternal()->hasFormatters()) {
        guiFactory()->addClient(Core::self()->sourceFormatterControllerInternal());
    }

    // Needed to re-plug the actions from the sessioncontroller as xmlguiclients don't
    // seem to remember which actions where plugged in.
    Core::self()->sessionController()->updateXmlGuiActionList();

    d->setupGui();

    qRegisterMetaType<QPointer<KTextEditor::Document>>();

    //Queued so we process it with some delay, to make sure the rest of the UI has already adapted
    connect(Core::self()->documentController(), &IDocumentController::documentActivated,
            // Use a queued connection, because otherwise the view is not yet fully set up
            // but wrap the document in a smart pointer to guard against crashes when it
            // gets deleted in the meantime
            this, [this](IDocument *doc) {
                const auto textDocument = QPointer<KTextEditor::Document>(doc->textDocument());
                QMetaObject::invokeMethod(this, "documentActivated", Qt::QueuedConnection,
                                          Q_ARG(QPointer<KTextEditor::Document>, textDocument));
            });

    connect(Core::self()->documentController(), &IDocumentController::documentClosed, this, &MainWindow::updateCaption, Qt::QueuedConnection);
    connect(Core::self()->documentController(), &IDocumentController::documentUrlChanged, this, &MainWindow::updateCaption, Qt::QueuedConnection);
    connect(Core::self()->documentController(), &IDocumentController::documentStateChanged, this, &MainWindow::updateCaption, Qt::QueuedConnection);
    connect(Core::self()->sessionController()->activeSession(), &ISession::sessionUpdated, this, &MainWindow::updateCaption);
    // if currently viewed document is part of project, trigger update of full path to project prefixed one
    connect(Core::self()->projectController(), &ProjectController::projectOpened, this, &MainWindow::updateCaption, Qt::QueuedConnection);

    connect(Core::self()->documentController(), &IDocumentController::documentOpened, this, &MainWindow::updateTabColor);
    connect(Core::self()->documentController(), &IDocumentController::documentUrlChanged, this, &MainWindow::updateTabColor);
    connect(this, &Sublime::MainWindow::viewAdded, this, &MainWindow::updateAllTabColors);
    connect(Core::self()->projectController(), &ProjectController::projectOpened, this, &MainWindow::updateAllTabColors, Qt::QueuedConnection);

    updateCaption();
}

void MainWindow::cleanup()
{
}

bool MainWindow::queryClose()
{
    if (!Core::self()->documentControllerInternal()->saveAllDocumentsForWindow(this, IDocument::Default))
        return false;

    return Sublime::MainWindow::queryClose();
}

void MainWindow::documentActivated(const QPointer<KTextEditor::Document>& textDocument)
{
    Q_D(MainWindow);

    updateCaption();

    // update active document connection
    disconnect(d->activeDocumentReadWriteConnection);
    if (textDocument) {
        d->activeDocumentReadWriteConnection = connect(textDocument, &KTextEditor::Document::readWriteChanged,
                                                    this, &MainWindow::updateCaption);
    }
}

void MainWindow::updateCaption()
{
    QString title;
    QString localFilePath;
    bool isDocumentModified = false;

    if(area()->activeView())
    {
        Sublime::Document* doc = area()->activeView()->document();
        auto* urlDoc = qobject_cast<Sublime::UrlDocument*>(doc);
        if(urlDoc)
        {
            if (urlDoc->url().isLocalFile()) {
                localFilePath = urlDoc->url().toLocalFile();
            }
            title += Core::self()->projectController()->prettyFileName(urlDoc->url(), KDevelop::IProjectController::FormatPlain);
        }
        else
            title += doc->title();

        auto iDoc = qobject_cast<IDocument*>(doc);
        if (iDoc && iDoc->textDocument() && !iDoc->textDocument()->isReadWrite()) {
            title += i18n(" (read only)");
        }

        title += QLatin1String(" [*]"); // [*] is placeholder for modified state, cmp. QWidget::windowModified

        isDocumentModified = iDoc && (iDoc->state() != IDocument::Clean);
    }

    const auto activeSession = Core::self()->sessionController()->activeSession();
    const QString sessionTitle = activeSession ? activeSession->description() : QString();
    if (!sessionTitle.isEmpty()) {
        if (title.isEmpty()) {
            title = sessionTitle;
        } else {
            title = sessionTitle + QLatin1String(" - [ ") + title + QLatin1Char(']');
        }
    }

    // Workaround for a bug observed on macOS with Qt 5.9.8 (TODO: test with newer Qt, report bug):
    // Ensure to call setCaption() (thus implicitly setWindowTitle()) before
    // setWindowModified() & setWindowFilePath().
    // Otherwise, if the state will change "modified" from true to false as well change the title string,
    // calling setWindowTitle() last results in the "modified" indicator==asterisk becoming part of the
    // displayed window title somehow.
    // Other platforms so far not known to be affected, any order of calls seems fine.
    setCaption(title);
    setWindowModified(isDocumentModified);
    setWindowFilePath(localFilePath);
}

void MainWindow::updateAllTabColors()
{
    auto documentController = Core::self()->documentController();
    if (!documentController)
        return;

    const auto defaultColor = ::defaultColor(palette());
    if (UiConfig::colorizeByProject()) {
        QHash<const Sublime::View*, QColor> viewColors;
        const auto containers = this->containers();
        for (auto* container : containers) {
            const auto views = container->views();
            viewColors.reserve(views.size());
            viewColors.clear();
            for (auto view : views) {
                const auto urlDoc = qobject_cast<Sublime::UrlDocument*>(view->document());
                if (urlDoc) {
                    viewColors[view] = colorForDocument(urlDoc->url(), palette(), defaultColor);
                }
            }
            container->setTabColors(viewColors);
        }
    } else {
        const auto containers = this->containers();
        for (auto* container : containers) {
            container->resetTabColors(defaultColor);
        }
    }
}

void MainWindow::updateTabColor(IDocument* doc)
{
    if (!UiConfig::self()->colorizeByProject())
        return;

    const auto color = colorForDocument(doc->url(), palette(), defaultColor(palette()));
    const auto containers = this->containers();
    for (auto* container : containers) {
        const auto views = container->views();
        for (auto* view : views) {
            const auto urlDoc = qobject_cast<Sublime::UrlDocument*>(view->document());
            if (urlDoc && urlDoc->url() == doc->url()) {
                container->setTabColor(view, color);
            }
        }
    }
}

void MainWindow::registerStatus(QObject* status)
{
    Q_D(MainWindow);

    d->registerStatus(status);
}

void MainWindow::initializeStatusBar()
{
    Q_D(MainWindow);

    d->setupStatusBar();
}

void MainWindow::showErrorMessage(const QString& message, int timeout)
{
    Q_D(MainWindow);

    d->showErrorMessage(message, timeout);
}

void MainWindow::tabContextMenuRequested(Sublime::View* view, QMenu* menu)
{
    Q_D(MainWindow);

    Sublime::MainWindow::tabContextMenuRequested(view, menu);
    d->tabContextMenuRequested(view, menu);
}

void MainWindow::tabToolTipRequested(Sublime::View* view, Sublime::Container* container, int tab)
{
    Q_D(MainWindow);

    d->tabToolTipRequested(view, container, tab);
}

void MainWindow::dockBarContextMenuRequested(Qt::DockWidgetArea area, const QPoint& position)
{
    Q_D(MainWindow);

    d->dockBarContextMenuRequested(area, position);
}

void MainWindow::newTabRequested()
{
    Q_D(MainWindow);

    Sublime::MainWindow::newTabRequested();

    d->fileNew();
}

#include "moc_mainwindow.cpp"
