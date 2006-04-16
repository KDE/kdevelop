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
#include <kicon.h>
#include <kedittoolbar.h>
#include <ktexteditor/view.h>
#include <ktexteditor/document.h>
#include <kapplication.h>

#include <ddockwidget.h>
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
    , m_currentTabDocument(0L)
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
            SIGNAL(documentUrlChanged( KDevDocument*, const KUrl &, const KUrl & )),
            this, SLOT(slotDocumentUrlChanged( KDevDocument*, const KUrl &, const KUrl & )));
    connect(DocumentController::getInstance(),
            SIGNAL(activePartChanged(KParts::Part*)),
            this, SLOT(activePartChanged(KParts::Part*)));
    connect(this, SIGNAL(widgetChanged(QWidget*)),
            this, SLOT(tabWidgetChanged(QWidget*)));

    connect(DocumentController::getInstance(),
            SIGNAL(documentStateChanged(KDevDocument*, KDevDocument::DocumentState)),
            this, SLOT(documentStateChanged(KDevDocument*, KDevDocument::DocumentState)));

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
    // Avoid recursive active part setting
    blockSignals(true);

    if (m_docks.contains(view))
    {
        DDockWidget *dock = toolWindow(m_docks[view]);
        dock->raiseWidget(view);
    }
    else if (m_widgets.contains(view) && m_widgetTabs.contains(view))
        m_widgetTabs[view]->setCurrentIndex(m_widgetTabs[view]->indexOf(view));

    blockSignals(false);
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
    m_raiseEditor = new KAction(i18n("Raise &Editor"), actionCollection(), "raise_editor");
    m_raiseEditor->setShortcut(Qt::ALT+Qt::Key_C);
    m_raiseEditor->setToolTip(i18n("Raise editor"));
    m_raiseEditor->setWhatsThis(i18n("<b>Raise editor</b><p>Focuses the editor."));
    connect(m_raiseEditor, SIGNAL(triggered(bool)), SLOT(raiseEditor()));

    KAction* splitHoriz = new KAction(i18n("Split &Horizontal"), actionCollection(), "split_h");
    splitHoriz->setShortcut(Qt::CTRL+Qt::SHIFT+Qt::Key_T);
    connect(splitHoriz, SIGNAL(triggered(bool)), SLOT(slotSplitHorizontal()));

    KAction* splitVert = new KAction(i18n("Split &Vertical"), actionCollection(), "split_v");
    splitVert->setShortcut(Qt::CTRL+Qt::SHIFT+Qt::Key_L);
    connect(splitVert, SIGNAL(triggered(bool)), SLOT(slotSplitVertical()));

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
    if ((m_activeTabWidget->currentIndex() + 1) < m_activeTabWidget->count())
        m_activeTabWidget->setCurrentIndex(m_activeTabWidget->currentIndex() + 1);
    else
        m_activeTabWidget->setCurrentIndex(0);
}

void SimpleMainWindow::gotoPreviousWindow()
{
    if ((m_activeTabWidget->currentIndex() - 1) >= 0)
        m_activeTabWidget->setCurrentIndex(m_activeTabWidget->currentIndex() - 1);
    else
        m_activeTabWidget->setCurrentIndex(m_activeTabWidget->count() - 1);
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

void SimpleMainWindow::slotDocumentUrlChanged( KDevDocument* document, const KUrl &oldURL, const KUrl &newURL )
{
    Q_UNUSED(oldURL)
    if (QWidget *widget = EditorProxy::getInstance()->topWidgetForPart(document->part()))
        widget->setWindowTitle(newURL.fileName());
}

void SimpleMainWindow::documentStateChanged(KDevDocument* document, KDevDocument::DocumentState state)
{
    QWidget * widget = EditorProxy::getInstance()->topWidgetForPart(document->part());
    if (widget)
    {
        switch (state)
        {
            // we should probably restore the original icon instead of just using "kdevelop",
            // but I have never seen any other icon in use so this should do for now
            case KDevDocument::Clean:
                if (m_showIconsOnTabs)
                    widget->setWindowIcon(KIcon("kdevelop"));
                else
                    widget->setWindowIcon(QPixmap());
                break;
            case KDevDocument::Modified:
                widget->setWindowIcon(KIcon("filesave"));
                break;
            case KDevDocument::Dirty:
                widget->setWindowIcon(KIcon("revert"));
                break;
            case KDevDocument::DirtyAndModified:
                widget->setWindowIcon(KIcon("stop"));
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
    tabMenu.addTitle(tabWidget->tabText(tabWidget->indexOf(w)));

    //Find the document on whose tab the user clicked
    m_currentTabDocument = 0L;
    foreach (KParts::Part* part, DocumentController::getInstance()->parts())
    {
        QWidget *top_widget = EditorProxy::getInstance()->topWidgetForPart(part);
        if (top_widget == w)
        {
            if (KParts::ReadOnlyPart *ro_part = qobject_cast<KParts::ReadOnlyPart*>(part))
            {
                m_currentTabDocument = DocumentController::getInstance()->documentForPart(ro_part);
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
                list << m_currentTabDocument->url();
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
    if(!m_currentTabDocument)
            return;

    switch(id)
    {
        case 0:
            DocumentController::getInstance()->closeDocument(m_currentTabDocument);
            break;
        case 1:
            DocumentController::getInstance()->saveDocument(m_currentTabDocument);
            break;
        case 2:
            DocumentController::getInstance()->reloadDocument(m_currentTabDocument);
            break;
        case 3:
            DocumentController::getInstance()->activateDocument(m_currentTabDocument);
            break;
        case 4:
            DocumentController::getInstance()->closeAllOthers(m_currentTabDocument);
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

    m_windowMenu->addAction(actionCollection()->action("file_close"));
    m_windowMenu->addAction(actionCollection()->action("file_close_all"));
    m_windowMenu->addAction(actionCollection()->action("file_closeother"));

    QObject::connect(m_windowMenu, SIGNAL(activated(int)), this, SLOT(openURL(int)));
    QObject::connect(m_windowMenu, SIGNAL(aboutToShow()), this, SLOT(fillWindowMenu()));
}

void SimpleMainWindow::openURL(int w)
{
    foreach(WinInfo pair, m_windowList) {
        if (pair.first == w) {
            if (pair.second) {
                pair.second->activate();
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

    QMap<QString, KDevDocument*> map;
    QStringList string_list;
    foreach (KDevDocument* file, DocumentController::getInstance()->openDocuments())
    {
        map[file->url().fileName()] = file;
        string_list.append(file->url().fileName());
    }
    string_list.sort();

    QList<KDevDocument*> list;
    for(uint i = 0; i != string_list.size(); ++i)
        list.append(map[string_list[i]]);

    if (list.count() > 0)
        m_windowList << qMakePair(m_windowMenu->insertSeparator(), static_cast<KDevDocument*>(0L));

    int i = 0;
    foreach (KDevDocument* file, list)
    {
        int temp = m_windowMenu->insertItem( i < 10 ? QString("&%1 %2").arg(i).arg(file->url().fileName()) : file->url().fileName() );
        m_windowList << qMakePair(temp, file);
        ++i;
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
    if ( KParts::Part *part = DocumentController::getInstance()->partForWidget( tabWidget ) )
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
