/***************************************************************************
 *   Copyright (C) 2005 by Alexander Dymo                                  *
 *   adymo@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.             *
 ***************************************************************************/
#include "simplemainwindow.h"

#include <QPixmap>
#include <qdebug.h>

#include <kaction.h>
#include <kstdaction.h>
#include <kparts/part.h>
#include <kmenu.h>
#include <kmenubar.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kedittoolbar.h>
#include <ktexteditor/view.h>
#include <ktexteditor/document.h>
#include <kapplication.h>

#include <ddockwindow.h>
#include <dtabwidget.h>
#include <profile.h>
#include <profileengine.h>
#include <kglobal.h>

#include "api.h"
#include "core.h"
#include "plugincontroller.h"
#include "mainwindowshare.h"
#include "shellextension.h"
#include "documentcontroller.h"
#include "statusbar.h"
#include "documentationpart.h"
#include "toplevel.h"
#include "projectmanager.h"
#include "editorproxy.h"

SimpleMainWindow::SimpleMainWindow(QWidget* parent)
    :DMainWindow(parent)
{
    resize(800, 600); // starts kdevelop at 800x600 the first time
    m_mainWindowShare = new MainWindowShare(this);
}

SimpleMainWindow::~ SimpleMainWindow( )
{
    TopLevel::invalidateInstance( this );
}

void SimpleMainWindow::init()
{
    setStandardToolBarMenuEnabled( true );
    setXMLFile(ShellExtension::getInstance()->xmlFile());

    createFramework();
    createActions();
    new KDevStatusBar(this);

    createGUI(0);

    m_mainWindowShare->init();
    setupWindowMenu();
    menuBar()->setEnabled( false );

    //FIXME: this checks only for global offers which is not quite correct because
    //a profile can offer core plugins and no global plugins.
    if ( PluginController::getInstance()->engine().allOffers(ProfileEngine::Global).isEmpty() )
    {
        KMessageBox::sorry( this, i18n("Unable to find plugins, KDevelop will not work"
            " properly.\nPlease make sure "
            "that KDevelop is installed in your KDE directory; otherwise, you have "
            "to add KDevelop's installation "
            "path to the environment variable KDEDIRS and run kbuildsycoca. Restart "
            "KDevelop afterwards.\n"
            "Example for BASH users:\nexport KDEDIRS=/path/to/kdevelop:$KDEDIRS && kbuildsycoca"),
            i18n("Could Not Find Plugins") );
    }

    connect(Core::getInstance(), SIGNAL(coreInitialized()), this, SLOT(slotCoreInitialized()));
    connect(Core::getInstance(), SIGNAL(projectOpened()), this, SLOT(projectOpened()));
    connect(DocumentController::getInstance(),
            SIGNAL(documentURLChanged( const KUrl &, const KUrl & )),
            this, SLOT(slotDocumentURLChanged( const KUrl &, const KUrl & )));
    connect(DocumentController::getInstance(),
            SIGNAL(activePartChanged(KParts::Part*)),
            this, SLOT(activePartChanged(KParts::Part*)));
    connect(this, SIGNAL(widgetChanged(QWidget*)),
            this, SLOT(tabWidgetChanged(QWidget*)));

    connect(DocumentController::getInstance(),
            SIGNAL(documentStateChanged(const KUrl &, DocumentState)),
            this, SLOT(documentStateChanged(const KUrl&, DocumentState)));

    loadSettings();
}

void SimpleMainWindow::embedPartView(QWidget *view, const QString &title, const QString &/*toolTip*/)
{
    if (!view)
        return;

    addWidget(view, title);
    view->show();
}

void SimpleMainWindow::embedSelectView(QWidget *view, const QString &title, const QString &/*toolTip*/)
{
    toolWindow(Qt::LeftDockWidgetArea)->addWidget(title, view);
    m_docks[view] = Qt::LeftDockWidgetArea;
}

void SimpleMainWindow::embedOutputView(QWidget *view, const QString &title, const QString &/*toolTip*/)
{
    toolWindow(Qt::BottomDockWidgetArea)->addWidget(title, view);
    m_docks[view] = Qt::BottomDockWidgetArea;
    toolWindow(Qt::BottomDockWidgetArea)->show();
}

void SimpleMainWindow::embedSelectViewRight(QWidget *view, const QString &title, const QString &/*toolTip*/)
{
    toolWindow(Qt::RightDockWidgetArea)->addWidget(title, view);
    m_docks[view] = Qt::RightDockWidgetArea;
}

void SimpleMainWindow::removeView(QWidget *view)
{
    if (!view)
        return;

    //try to remove it from all parts of main window
    //@fixme This method needs to be divided in two - one for docks and one for part views
    if (m_docks.contains(view))
        toolWindow(m_docks[view])->removeWidget(view);
    else
        removeWidget(view);
}

void SimpleMainWindow::setViewAvailable(QWidget *pView, bool bEnabled)
{
    DDockWidget *dock;
    if (m_docks.contains(pView))
        dock = toolWindow(m_docks[pView]);
    else
        return;

    bEnabled ? dock->showWidget(pView) : dock->hideWidget(pView);
}

void SimpleMainWindow::raiseView(QWidget *view)
{
    //adymo: a workaround to make editor wrappers work:
    //editor view is passed to this function but the ui library knows only
    //of its parent which is an editor wrapper, simply replacing the view
    //by its wrapper helps here
    if (view->parent() && view->parent()->isA("EditorWrapper"))
    {
//         kDebug() << "parent is editor wrapper: " <<
//             static_cast<EditorWrapper*>(view->parent()) << endl;
        view = (QWidget*)view->parent();
    }

    if (m_docks.contains(view))
    {
        DDockWidget *dock = toolWindow(m_docks[view]);
        dock->raiseWidget(view);
    }
    else if (m_widgets.contains(view) && m_widgetTabs.contains(view))
        m_widgetTabs[view]->showPage(view);
}

void SimpleMainWindow::lowerView(QWidget * /*view*/)
{
    //nothing to do
}

void SimpleMainWindow::loadSettings()
{
    KConfig *config = KGlobal::config();

    ProjectManager::getInstance()->loadSettings();
    applyMainWindowSettings(config, QLatin1String("SimpleMainWindow"));
}

void SimpleMainWindow::saveSettings( )
{
    KConfig *config = KGlobal::config();

    ProjectManager::getInstance()->saveSettings();
    saveMainWindowSettings(config, QLatin1String("SimpleMainWindow"));
}

KMainWindow *SimpleMainWindow::main()
{
    return this;
}

void SimpleMainWindow::createFramework()
{
    DocumentController::createInstance( this );

    connect(DocumentController::getInstance(),
            SIGNAL(activePartChanged(KParts::Part*)),
            this, SLOT(createGUI(KParts::Part*)));
}

void SimpleMainWindow::createActions()
{
    m_raiseEditor = new KAction(i18n("Raise &Editor"), Qt::ALT+Qt::Key_C,
        this, SLOT(raiseEditor()), actionCollection(), "raise_editor");
    m_raiseEditor->setToolTip(i18n("Raise editor"));
    m_raiseEditor->setWhatsThis(i18n("<b>Raise editor</b><p>Focuses the editor."));

    new KAction(i18n("Split &Horizontal"), Qt::CTRL+Qt::SHIFT+Qt::Key_T,
        this, SLOT(slotSplitHorizontal()), actionCollection(), "split_h");

    new KAction(i18n("Split &Vertical"), Qt::CTRL+Qt::SHIFT+Qt::Key_L,
        this, SLOT(slotSplitVertical()), actionCollection(), "split_v");

    KStdAction::configureToolbars(this, SLOT(configureToolbars()),
        actionCollection(), "set_configure_toolbars");

    m_mainWindowShare->createActions();

    connect(m_mainWindowShare, SIGNAL(gotoNextWindow()), this, SLOT(gotoNextWindow()));
    connect(m_mainWindowShare, SIGNAL(gotoPreviousWindow()), this, SLOT(gotoPreviousWindow()));
    connect(m_mainWindowShare, SIGNAL(gotoFirstWindow()), this, SLOT(gotoFirstWindow()));
    connect(m_mainWindowShare, SIGNAL(gotoLastWindow()), this, SLOT(gotoLastWindow()));
}

void SimpleMainWindow::raiseEditor()
{
    KDevDocumentController *partcontroller = API::getInstance()->documentController();
    if (partcontroller->activePart() && partcontroller->activePart()->widget())
        partcontroller->activePart()->widget()->setFocus();
}

void SimpleMainWindow::gotoNextWindow()
{
    if ((m_activeTabWidget->currentPageIndex() + 1) < m_activeTabWidget->count())
        m_activeTabWidget->setCurrentPage(m_activeTabWidget->currentPageIndex() + 1);
    else
        m_activeTabWidget->setCurrentPage(0);
}

void SimpleMainWindow::gotoPreviousWindow()
{
    if ((m_activeTabWidget->currentPageIndex() - 1) >= 0)
        m_activeTabWidget->setCurrentPage(m_activeTabWidget->currentPageIndex() - 1);
    else
        m_activeTabWidget->setCurrentPage(m_activeTabWidget->count() - 1);
}

void SimpleMainWindow::gotoFirstWindow()
{
    //@todo implement
}

void SimpleMainWindow::gotoLastWindow()
{
    //@todo implement
}

void SimpleMainWindow::slotCoreInitialized()
{
    menuBar()->setEnabled(true);
}

void SimpleMainWindow::projectOpened()
{
    setCaption(QString());
}

void SimpleMainWindow::slotDocumentURLChanged( const KUrl &oldURL, const KUrl &newURL )
{
    if (QWidget *widget = EditorProxy::getInstance()->topWidgetForPart(
        DocumentController::getInstance()->partForURL(newURL)))
        widget->setCaption(newURL.fileName());
}

void SimpleMainWindow::documentStateChanged(const KUrl &url, DocumentState state)
{
    QWidget * widget = EditorProxy::getInstance()->topWidgetForPart(
        DocumentController::getInstance()->partForURL(url));
    if (widget)
    {
        //calculate the icon size if showTabIcons is false
        //this is necessary to avoid tab resizing by setIcon() call
        int isize = 16;
        if (m_activeTabWidget && !m_showIconsOnTabs)
        {
            isize = m_activeTabWidget->fontMetrics().height() - 1;
            isize = isize > 16 ? 16 : isize;
        }
        switch (state)
        {
            // we should probably restore the original icon instead of just using "kdevelop",
            // but I have never seen any other icon in use so this should do for now
            case Clean:
                if (m_showIconsOnTabs)
                    widget->setIcon(SmallIcon("kdevelop", isize));
                else
                    widget->setIcon(QPixmap());
                break;
            case Modified:
                widget->setIcon(SmallIcon("filesave", isize));
                break;
            case Dirty:
                widget->setIcon(SmallIcon("revert", isize));
                break;
            case DirtyAndModified:
                widget->setIcon(SmallIcon("stop", isize));
                break;
        }
    }
}

void SimpleMainWindow::closeTab()
{
    actionCollection()->action("file_close")->trigger();
}

void SimpleMainWindow::tabContext(QWidget *w, const QPoint &p)
{
    DTabWidget *tabWidget = qobject_cast<DTabWidget *>(sender());
    if (!tabWidget)
        return;

    KMenu tabMenu;
    tabMenu.addTitle(tabWidget->tabLabel(w));

    //Find the document on whose tab the user clicked
    m_currentTabURL = QString();
    foreach (KParts::Part* part, DocumentController::getInstance()->parts())
    {
        QWidget *top_widget = EditorProxy::getInstance()->topWidgetForPart(part);
        if (top_widget == w)
        {
            if (KParts::ReadOnlyPart *ro_part = qobject_cast<KParts::ReadOnlyPart*>(part))
            {
                m_currentTabURL = ro_part->url();
                tabMenu.insertItem(i18n("Close"), 0);

                if (DocumentController::getInstance()->parts().count() > 1)
                    tabMenu.insertItem(i18n("Close All Others"), 4);

                if (qobject_cast<HTMLDocumentationPart*>(ro_part))
                {
                    tabMenu.insertItem(i18n("Duplicate"), 3);
                    break;
                }

                //Create the file context
                KUrl::List list;
                list << m_currentTabURL;
                FileContext context( list );
                Core::getInstance()->fillContextMenu(&tabMenu, &context);
            }
            break;
        }
    }

    connect(&tabMenu, SIGNAL(activated(int)), this, SLOT(tabContextActivated(int)));
    tabMenu.exec(p);
}

void SimpleMainWindow::tabContextActivated(int id)
{
    if(m_currentTabURL.isEmpty())
            return;

    switch(id)
    {
        case 0:
            DocumentController::getInstance()->closeDocument(m_currentTabURL);
            break;
        case 1:
            DocumentController::getInstance()->saveDocument(m_currentTabURL);
            break;
        case 2:
            DocumentController::getInstance()->reloadDocument(m_currentTabURL);
            break;
        case 3:
            DocumentController::getInstance()->showDocument(m_currentTabURL, true);
            break;
        case 4:
            DocumentController::getInstance()->closeAllOthers(m_currentTabURL);
            break;
        default:
            break;
    }
}

void SimpleMainWindow::configureToolbars()
{
    saveMainWindowSettings(KGlobal::config(), QLatin1String("SimpleMainWindow"));
    KEditToolbar dlg(factory());
    connect(&dlg, SIGNAL(newToolbarConfig()), this, SLOT(slotNewToolbarConfig()));
    dlg.exec();
}

void SimpleMainWindow::slotNewToolbarConfig()
{
//    setupWindowMenu();
    m_mainWindowShare->slotGUICreated(DocumentController::getInstance()->activePart());
    applyMainWindowSettings(KGlobal::config(), QLatin1String("SimpleMainWindow"));
}

bool SimpleMainWindow::queryClose()
{
    saveSettings();
    return Core::getInstance()->queryClose();
}

bool SimpleMainWindow::queryExit()
{
    return true;
}

void SimpleMainWindow::setupWindowMenu()
{
    // get the xmlgui created one instead
    m_windowMenu = qFindChild<KMenu *>(main(), QLatin1String("window"));

    if (!m_windowMenu)
    {
        kDebug(9000) << "Couldn't find the XMLGUI window menu. Creating new." << endl;

        m_windowMenu = new KMenu(main());
        menuBar()->insertItem(i18n("&Window"), m_windowMenu);
    }

    actionCollection()->action("file_close")->plug(m_windowMenu);
    actionCollection()->action("file_close_all")->plug(m_windowMenu);
    actionCollection()->action("file_closeother")->plug(m_windowMenu);

    QObject::connect(m_windowMenu, SIGNAL(activated(int)), this, SLOT(openURL(int)));
    QObject::connect(m_windowMenu, SIGNAL(aboutToShow()), this, SLOT(fillWindowMenu()));
}

void SimpleMainWindow::openURL(int w)
{
    foreach(WinInfo pair, m_windowList) {
        if (pair.first == w) {
            if (!pair.second.isEmpty()) {
                DocumentController::getInstance()->editDocument(pair.second);
                return;
            }
        }
    }
}

void SimpleMainWindow::fillWindowMenu()
{
    // clear menu
    foreach(WinInfo pair, m_windowList) {
        m_windowMenu->removeItem(pair.first);
    }

    int temp = 0;

    QMap<QString, KUrl> map;
    QStringList string_list;
    KUrl::List list = DocumentController::getInstance()->openURLs();
    KUrl::List::Iterator itt = list.begin();
    while (itt != list.end())
    {
        map[(*itt).fileName()] = *itt;
        string_list.append((*itt).fileName());
        ++itt;
    }
    string_list.sort();

    list.clear();
    for(uint i = 0; i != string_list.size(); ++i)
        list.append(map[string_list[i]]);

    itt = list.begin();
    int i = 0;

    if (list.count() > 0)
        m_windowList << qMakePair(m_windowMenu->insertSeparator(), KUrl());

    while (itt != list.end())
    {
        temp = m_windowMenu->insertItem( i < 10 ? QString("&%1 %2").arg(i).arg((*itt).fileName()) : (*itt).fileName() );
        m_windowList << qMakePair(temp, *itt);
        ++i;
        ++itt;
    }
}

void SimpleMainWindow::slotSplitVertical()
{
    DTabWidget *tab = splitVertical();
    DocumentController::getInstance()->openEmptyTextDocument();

    //FIXME: adymo: we can't put another kate view into the tab just added - weird crashes :(
    //more: kdevelop part controller doesn't handle such situation - it assumes the part to
    //have only one widget
/*    KParts::Part *activePart = DocumentController::getInstance()->activePart();
    if (!activePart)
        return;
    KTextEditor::Document *activeDoc = dynamic_cast<KTextEditor::Document *>(activePart);
    if (!activeDoc)
        return;

    QWidget *view = activeDoc->createView(0);
    addWidget(tab, view, "");
    view->show();*/
}

void SimpleMainWindow::slotSplitHorizontal()
{
    DTabWidget *tab = splitHorizontal();
    DocumentController::getInstance()->openEmptyTextDocument();
}

void SimpleMainWindow::tabWidgetChanged( QWidget *tabWidget )
{
    if ( KParts::Part *part =
            DocumentController::getInstance()->partForWidget( tabWidget ) )
    DocumentController::getInstance()->activatePart( part );
}

void SimpleMainWindow::closeTab(QWidget *w)
{
    foreach (KParts::Part* part, DocumentController::getInstance()->parts())
    {
        QWidget *widget = EditorProxy::getInstance()->topWidgetForPart(part);
        if (widget && widget == w)
        {
            DocumentController::getInstance()->closePart(part);
            return;
        }
    }
}

void SimpleMainWindow::activePartChanged(KParts::Part *part)
{
    if (!part)
        return;
    QWidget *w = part->widget();
    kDebug() << "active part widget is : " << w << endl;
    if (!m_widgets.contains(w))
        return;
    if (m_widgetTabs[w] != 0)
    {
        kDebug() << " setting m_activeTabWidget " << endl;
        m_activeTabWidget = m_widgetTabs[w];
    }
}

#include "simplemainwindow.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
