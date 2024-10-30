/*
    SPDX-FileCopyrightText: 2002 Falk Brettschneider <falkbr@kdevelop.org>
    SPDX-FileCopyrightText: 2003 John Firebaugh <jfirebaugh@kde.org>
    SPDX-FileCopyrightText: 2006 Adam Treat <treat@kde.org>
    SPDX-FileCopyrightText: 2006, 2007 Alexander Dymo <adymo@kdevelop.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "mainwindow_p.h"

#include <QApplication>
#include <QLayout>
#include <QMenu>

#include <KActionCollection>
#include <KActionMenu>
#include <KColorSchemeManager>
#include <KColorSchemeMenu>
#include <KStandardAction>
#include <KXMLGUIClient>
#include <KXMLGUIFactory>

#include <sublime/area.h>
#include <sublime/view.h>
#include <sublime/document.h>
#include <sublime/tooldocument.h>

#include <util/pushvalue.h>

#include <interfaces/iplugin.h>

#include "core.h"
#include "partdocument.h"
#include "partcontroller.h"
#include "uicontroller.h"
#include "statusbar.h"
#include "mainwindow.h"
#include "textdocument.h"
#include "sessioncontroller.h"
#include "sourceformattercontroller.h"
#include "debug.h"
#include "ktexteditorpluginintegration.h"

#include <language/duchain/duchainlock.h>
#include <language/duchain/duchainutils.h>
#include <language/duchain/topducontext.h>
#include <sublime/container.h>
#include <sublime/holdupdates.h>


namespace KDevelop {

MainWindowPrivate::MainWindowPrivate(MainWindow *mainWindow)
    : QObject(mainWindow)
    , m_mainWindow(mainWindow)
    , m_statusBar(nullptr)
    , lastXMLGUIClientView(nullptr)
    , m_changingActiveView(false)
    , m_kateWrapper(new KTextEditorIntegration::MainWindow(mainWindow))
{
}

bool MainWindowPrivate::isGuiSetUp() const
{
    return m_isGuiSetUp;
}

void MainWindowPrivate::setupGui()
{
    m_statusBar = new KDevelop::StatusBar(m_mainWindow);
    setupStatusBar();
    m_isGuiSetUp = true;
}

void MainWindowPrivate::setupStatusBar()
{
    QWidget *location = m_mainWindow->statusBarLocation();
    if (m_statusBar)
        location->layout()->addWidget(m_statusBar);
}

void MainWindowPrivate::addPlugin( IPlugin *plugin )
{
    qCDebug(SHELL) << "add plugin" << plugin << plugin->componentName();
    Q_ASSERT( plugin );

    //The direct plugin client can only be added to the first mainwindow
    if(m_mainWindow == Core::self()->uiControllerInternal()->mainWindows()[0])
        m_mainWindow->guiFactory()->addClient( plugin );

    Q_ASSERT(!m_pluginCustomClients.contains(plugin));

    KXMLGUIClient* ownClient = plugin->createGUIForMainWindow(m_mainWindow);
    if(ownClient) {
        m_pluginCustomClients[plugin] = ownClient;
        connect(plugin, &IPlugin::destroyed, this, &MainWindowPrivate::pluginDestroyed);
        m_mainWindow->guiFactory()->addClient(ownClient);
    }
}

void MainWindowPrivate::pluginDestroyed(QObject* pluginObj)
{
    auto* plugin = static_cast<IPlugin*>(pluginObj);
    KXMLGUIClient* p = m_pluginCustomClients.take(plugin);
    m_mainWindow->guiFactory()->removeClient( p );
    delete p;
}

MainWindowPrivate::~MainWindowPrivate()
{
    qDeleteAll(m_pluginCustomClients);
}

void MainWindowPrivate::removePlugin( IPlugin *plugin )
{
    Q_ASSERT( plugin );

    pluginDestroyed(plugin);
    disconnect(plugin, &IPlugin::destroyed, this, &MainWindowPrivate::pluginDestroyed);

    m_mainWindow->guiFactory()->removeClient( plugin );
}

void MainWindowPrivate::updateSourceFormatterGuiClient(bool hasFormatters)
{
    auto sourceFormatterController = Core::self()->sourceFormatterControllerInternal();
    auto guiFactory = m_mainWindow->guiFactory();
    if (hasFormatters) {
        guiFactory->addClient(sourceFormatterController);
    } else {
        guiFactory->removeClient(sourceFormatterController);
    }
}

void MainWindowPrivate::activePartChanged(KParts::Part *part)
{
    if ( Core::self()->uiController()->activeMainWindow() == m_mainWindow)
        m_mainWindow->createGUI(part);
}

void MainWindowPrivate::changeActiveView(Sublime::View *view)
{
    //disable updates on a window to avoid toolbar flickering on xmlgui client change
    Sublime::HoldUpdates s(m_mainWindow);
    mergeView(view);

    if(!view)
        return;

    auto* doc = qobject_cast<KDevelop::IDocument*>(view->document());
    if (doc)
    {
        doc->activate(view, m_mainWindow);
    }
    else
    {
        //activated view is not a part document so we need to remove active part gui
        ///@todo adymo: only this window needs to remove GUI
//         KParts::Part *activePart = Core::self()->partController()->activePart();
//         if (activePart)
//             guiFactory()->removeClient(activePart);
    }
}

void MainWindowPrivate::mergeView(Sublime::View* view)
{
    PushPositiveValue<bool> block(m_changingActiveView, true);

    // If the previous view was KXMLGUIClient, remove its actions
    // In the case that that view was removed, lastActiveView
    // will auto-reset, and xmlguifactory will disconnect that
    // client, I think.
    if (lastXMLGUIClientView)
    {
        qCDebug(SHELL) << "clearing last XML GUI client" << lastXMLGUIClientView;

        m_mainWindow->guiFactory()->removeClient(dynamic_cast<KXMLGUIClient*>(lastXMLGUIClientView));

        disconnect (lastXMLGUIClientView, &QWidget::destroyed, this, nullptr);

        lastXMLGUIClientView = nullptr;
    }

    if (!view)
        return;

    QWidget* viewWidget = view->widget();
    Q_ASSERT(viewWidget);

    qCDebug(SHELL) << "changing active view to" << view << "doc" << view->document() << "mw" << m_mainWindow;

    // If the new view is KXMLGUIClient, add it.
    if (auto* c = dynamic_cast<KXMLGUIClient*>(viewWidget))
    {
        qCDebug(SHELL) << "setting new XMLGUI client" << viewWidget;
        lastXMLGUIClientView = viewWidget;
        m_mainWindow->guiFactory()->addClient(c);
        connect(viewWidget, &QWidget::destroyed,
                this, &MainWindowPrivate::xmlguiclientDestroyed);
    }
}

void MainWindowPrivate::xmlguiclientDestroyed(QObject* obj)
{
    /* We're informed the QWidget for the active view that is also
       KXMLGUIclient is dying.  KXMLGUIFactory will not like deleted
       clients, really.  Unfortunately, there's nothing we can do
       at this point. For example, KateView derives from QWidget and
       KXMLGUIClient.  The destroyed() signal is emitted by ~QWidget.
       At this point, event attempt to cross-cast to KXMLGUIClient
       is undefined behaviour.  We hope to catch view deletion a bit
       later, but if we fail, we better report it now, rather than
       get a weird crash a bit later.  */
       Q_ASSERT(obj == lastXMLGUIClientView);
       Q_ASSERT(false && "xmlgui clients management is messed up");
       Q_UNUSED(obj);
}

void MainWindowPrivate::setupActions()
{
    connect(Core::self()->sessionController(), &SessionController::quitSession, this, &MainWindowPrivate::quitAll);

    QAction* action;

    const QString app = qApp->applicationName();
    action = KStandardAction::preferences( this, SLOT(settingsDialog()),
                                      actionCollection());
    action->setToolTip( i18nc( "@info:tooltip %1 = application name", "Configure %1", app ) );
    action->setWhatsThis( i18nc("@info:whatsthis", "Lets you customize %1.", app ) );

    action =  KStandardAction::configureNotifications(this, SLOT(configureNotifications()), actionCollection());
    action->setText( i18nc("@action", "Configure Notifications...") );
    action->setToolTip( i18nc("@info:tooltip", "Configure notifications") );
    action->setWhatsThis( i18nc( "@info:whatsthis", "Shows a dialog that lets you configure notifications." ) );

    action = actionCollection()->addAction( QStringLiteral("loaded_plugins"), this, SLOT(showLoadedPlugins()) );
    action->setIcon(QIcon::fromTheme(QStringLiteral("plugins")));
    action->setText( i18nc("@action", "Loaded Plugins") );
    action->setToolTip( i18nc("@info:tooltip", "Show a list of all loaded plugins") );
    action->setWhatsThis( i18nc( "@info:whatsthis", "Shows a dialog with information about all loaded plugins." ) );

    action = actionCollection()->addAction( QStringLiteral("view_next_window") );
    action->setText( i18nc("@action", "&Next Window" ) );
    connect( action, &QAction::triggered, this, &MainWindowPrivate::gotoNextWindow );
    actionCollection()->setDefaultShortcut(action, Qt::ALT | Qt::SHIFT | Qt::Key_Right);
    action->setToolTip( i18nc( "@info:tooltip", "Next window" ) );
    action->setWhatsThis( i18nc( "@info:whatsthis", "Switches to the next window." ) );
    action->setIcon(QIcon::fromTheme(QStringLiteral("go-next")));

    action = actionCollection()->addAction( QStringLiteral("view_previous_window") );
    action->setText( i18nc("@action", "&Previous Window" ) );
    connect( action, &QAction::triggered, this, &MainWindowPrivate::gotoPreviousWindow );
    actionCollection()->setDefaultShortcut(action, Qt::ALT | Qt::SHIFT | Qt::Key_Left);
    action->setToolTip( i18nc( "@info:tooltip", "Previous window" ) );
    action->setWhatsThis( i18nc( "@info:whatsthis", "Switches to the previous window." ) );
    action->setIcon(QIcon::fromTheme(QStringLiteral("go-previous")));

    action = actionCollection()->addAction(QStringLiteral("next_error"));
    action->setText(i18nc("@action", "Jump to Next Outputmark"));
    actionCollection()->setDefaultShortcut( action, QKeySequence(Qt::Key_F4) );
    action->setIcon(QIcon::fromTheme(QStringLiteral("arrow-right")));
    connect(action, &QAction::triggered, this, &MainWindowPrivate::selectNextItem);

    action = actionCollection()->addAction(QStringLiteral("prev_error"));
    action->setText(i18nc("@action", "Jump to Previous Outputmark"));
    actionCollection()->setDefaultShortcut( action, QKeySequence(Qt::SHIFT | Qt::Key_F4) );
    action->setIcon(QIcon::fromTheme(QStringLiteral("arrow-left")));
    connect(action, &QAction::triggered, this, &MainWindowPrivate::selectPrevItem);

    action = actionCollection()->addAction( QStringLiteral("split_horizontal") );
    action->setIcon(QIcon::fromTheme( QStringLiteral("view-split-top-bottom") ));
    action->setText( i18nc("@action", "Split View &Top/Bottom" ) );
    actionCollection()->setDefaultShortcut(action, Qt::CTRL | Qt::SHIFT | Qt::Key_T);
    connect( action, &QAction::triggered, this, &MainWindowPrivate::splitHorizontal );
    action->setToolTip( i18nc( "@info:tooltip", "Split horizontal" ) );
    action->setWhatsThis( i18nc( "@info:whatsthis", "Splits the current view horizontally." ) );

    action = actionCollection()->addAction( QStringLiteral("split_vertical") );
    action->setIcon(QIcon::fromTheme( QStringLiteral("view-split-left-right") ));
    action->setText( i18nc("@action", "Split View &Left/Right" ) );
    actionCollection()->setDefaultShortcut(action, Qt::CTRL | Qt::SHIFT | Qt::Key_L);
    connect( action, &QAction::triggered, this, &MainWindowPrivate::splitVertical );
    action->setToolTip( i18nc( "@info:tooltip", "Split vertical" ) );
    action->setWhatsThis( i18nc( "@info:whatsthis", "Splits the current view vertically." ) );

    action = actionCollection()->addAction( QStringLiteral("view_next_split") );
    action->setText( i18nc("@action", "&Next Split View" ) );
    connect( action, &QAction::triggered, this, &MainWindowPrivate::gotoNextSplit );
    actionCollection()->setDefaultShortcut(action, Qt::CTRL | Qt::SHIFT | Qt::Key_N);
    action->setToolTip( i18nc( "@info:tooltip", "Next split view" ) );
    action->setWhatsThis( i18nc( "@info:whatsthis", "Switches to the next split view." ) );
    action->setIcon(QIcon::fromTheme(QStringLiteral("go-next")));

    action = actionCollection()->addAction( QStringLiteral("view_previous_split") );
    action->setText( i18nc("@action", "&Previous Split View" ) );
    connect( action, &QAction::triggered, this, &MainWindowPrivate::gotoPreviousSplit );
    actionCollection()->setDefaultShortcut(action, Qt::CTRL | Qt::SHIFT | Qt::Key_P);
    action->setToolTip( i18nc( "@info:tooltip", "Previous split view" ) );
    action->setWhatsThis( i18nc( "@info:whatsthis", "Switches to the previous split view." ) );
    action->setIcon(QIcon::fromTheme(QStringLiteral("go-previous")));

    KStandardAction::fullScreen( this, SLOT(toggleFullScreen(bool)), m_mainWindow, actionCollection() );

    action = actionCollection()->addAction( QStringLiteral("file_new") );
    action->setIcon(QIcon::fromTheme(QStringLiteral("document-new")));
    actionCollection()->setDefaultShortcut(action, Qt::CTRL | Qt::Key_N);
    action->setText( i18nc("@action new file", "&New" ) );
    action->setIconText( i18nc( "@action Shorter Text for 'New File' shown in the toolbar", "New") );
    connect( action, &QAction::triggered, this, &MainWindowPrivate::fileNew );
    action->setToolTip( i18nc( "@info:tooltip", "New file" ) );
    action->setWhatsThis( i18nc( "@info:whatsthis", "Creates an empty file." ) );

    action = actionCollection()->addAction( QStringLiteral("add_toolview") );
    action->setIcon(QIcon::fromTheme(QStringLiteral("window-new")));
    actionCollection()->setDefaultShortcut(action, Qt::CTRL | Qt::SHIFT | Qt::Key_V);
    action->setText( i18nc("@action", "&Add Tool View..." ) );
    connect( action, &QAction::triggered,  this, &MainWindowPrivate::viewAddNewToolView );
    action->setToolTip( i18nc( "@info:tooltip", "Add tool view" ) );
    action->setWhatsThis( i18nc( "@info:whatsthis", "Adds a new tool view to this window." ) );

    //Load themes
    auto* const manager = new KColorSchemeManager(this);
    auto* const colorSelectionMenu = KColorSchemeMenu::createMenu(manager, actionCollection());
    colorSelectionMenu->menu()->setTitle(i18n("&Window Color Scheme"));
    actionCollection()->addAction(QStringLiteral("colorscheme_menu"), colorSelectionMenu);
}

void MainWindowPrivate::toggleArea(bool b)
{
    if (!b) return;
    auto* action = qobject_cast<QAction*>(sender());
    if (!action) return;
    m_mainWindow->controller()->showArea(action->data().toString(), m_mainWindow);
}

KActionCollection * MainWindowPrivate::actionCollection()
{
    return m_mainWindow->actionCollection();
}

void MainWindowPrivate::registerStatus(QObject* status)
{
    m_statusBar->registerStatus(status);
}


void MainWindowPrivate::showErrorMessage(const QString& message, int timeout)
{
    m_statusBar->showErrorMessage(message, timeout);
}

void MainWindowPrivate::tabContextMenuRequested(Sublime::View* view, QMenu* menu)
{
    m_tabView = view;

    QAction* action;

    action = menu->addAction(QIcon::fromTheme(QStringLiteral("view-split-top-bottom")), i18nc("@action:inmenu", "Split View Top/Bottom"));
    connect(action, &QAction::triggered, this, &MainWindowPrivate::contextMenuSplitHorizontal);

    action = menu->addAction(QIcon::fromTheme(QStringLiteral("view-split-left-right")), i18nc("@action:inmenu", "Split View Left/Right"));
    connect(action, &QAction::triggered, this, &MainWindowPrivate::contextMenuSplitVertical);
    menu->addSeparator();

    action = menu->addAction(QIcon::fromTheme(QStringLiteral("document-new")), i18nc("@action:inmenu", "New File"));

    connect(action, &QAction::triggered, this, &MainWindowPrivate::contextMenuFileNew);

    if (view) {
        if (auto* doc = qobject_cast<TextDocument*>(view->document())) {
            action = menu->addAction(QIcon::fromTheme(QStringLiteral("view-refresh")), i18nc("@action:inmenu", "Reload"));
            connect(action, &QAction::triggered, doc, &TextDocument::reload);

            action = menu->addAction(QIcon::fromTheme(QStringLiteral("view-refresh")), i18nc("@action:inmenu", "Reload All"));
            connect(action, &QAction::triggered, this, &MainWindowPrivate::reloadAll);
        }
    }
}

void MainWindowPrivate::tabToolTipRequested(Sublime::View* view, Sublime::Container* container, int tab)
{
    if (m_tabTooltip.second) {
        if (m_tabTooltip.first == view) {
            // tooltip already shown, don't do anything. prevents flicker when moving mouse over same tab
            return;
        } else {
            m_tabTooltip.second.data()->close();
        }
    }

    auto* urlDoc = qobject_cast<Sublime::UrlDocument*>(view->document());

    if (urlDoc) {
        DUChainReadLocker lock;
        TopDUContext* top = DUChainUtils::standardContextForUrl(urlDoc->url());

        if (top) {
            if (auto* navigationWidget = top->createNavigationWidget()) {
                auto* tooltip = new KDevelop::NavigationToolTip(m_mainWindow, QCursor::pos() + QPoint(20, 20), navigationWidget);
                tooltip->resize(navigationWidget->sizeHint() + QSize(10, 10));
                tooltip->setHandleRect(container->tabRect(tab));

                m_tabTooltip.first = view;
                m_tabTooltip.second = tooltip;
                ActiveToolTip::showToolTip(m_tabTooltip.second.data());
            }
        }
    }
}

void MainWindowPrivate::dockBarContextMenuRequested(Qt::DockWidgetArea area, const QPoint& position)
{
    QMenu menu(m_mainWindow);
    menu.addSection(QIcon::fromTheme(QStringLiteral("window-new")), i18nc("@action:inmenu", "Add Tool View"));
    QHash<IToolViewFactory*, Sublime::ToolDocument*> factories =
        Core::self()->uiControllerInternal()->factoryDocuments();
    QHash<QAction*, IToolViewFactory*> actionToFactory;
    if ( !factories.isEmpty() ) {
        // sorted actions
        QMap<QString, QAction*> actionMap;
        for (QHash<IToolViewFactory*, Sublime::ToolDocument*>::const_iterator it = factories.constBegin();
                it != factories.constEnd(); ++it)
        {
            auto* action = new QAction(it.value()->statusIcon(), it.value()->title(), &menu);
            action->setIcon(it.value()->statusIcon());
            if (!it.key()->allowMultiple() && Core::self()->uiControllerInternal()->toolViewPresent(it.value(), m_mainWindow->area())) {
                action->setDisabled(true);
            }
            actionToFactory.insert(action, it.key());
            actionMap[action->text()] = action;
        }
        menu.addActions(actionMap.values());
    }

    auto* const lockAction = new QAction(&menu);
    lockAction->setCheckable(true);
    lockAction->setText(i18nc("@action:inmenu", "Lock the Panel from Hiding"));

    KConfigGroup config = KSharedConfig::openConfig()->group(QStringLiteral("UI"));
    lockAction->setChecked(config.readEntry(QStringLiteral("Toolview Bar (%1) Is Locked").arg(area), false));

    menu.addSeparator();
    menu.addAction(lockAction);

    QAction* triggered = menu.exec(position);
    if ( !triggered ) {
        return;
    }

    if (triggered == lockAction) {
        KConfigGroup config = KSharedConfig::openConfig()->group(QStringLiteral("UI"));
        config.writeEntry(QStringLiteral("Toolview Bar (%1) Is Locked").arg(area), lockAction->isChecked());
        return;
    }

    Core::self()->uiControllerInternal()->addToolViewToDockArea(
        actionToFactory[triggered],
        area
    );
}

bool MainWindowPrivate::changingActiveView() const
{
    return m_changingActiveView;
}

KTextEditorIntegration::MainWindow *MainWindowPrivate::kateWrapper() const
{
    return m_kateWrapper;
}

}

#include "mainwindow_actions.cpp"
#include "moc_mainwindow_p.cpp"
