/***************************************************************************
  mainwindow.cpp - KDevelop main widget for all QextMDI-based user
                   interface modes (Childframe, TabPage, Toplevel)
                             -------------------
    begin                : 22 Dec 2002
    copyright            : (C) 2002 by the KDevelop team
    email                : team@kdevelop.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "mainwindow.h"

#include <qlayout.h>
#include <qmultilineedit.h>
#include <qvbox.h>
#include <qcheckbox.h>
#include <qvaluelist.h>
#include <qguardedptr.h>

#include <kdeversion.h>
#include <kapplication.h>
#include <kstdaction.h>
#include <kdebug.h>
#include <kaction.h>
#include <klocale.h>
#include <kconfig.h>
#include <kstatusbar.h>
#include <kdialogbase.h>
#include <kkeydialog.h>
#include <kedittoolbar.h>
#include <kmenubar.h>
#include <kmessagebox.h>
#include <kiconloader.h>

#include <kdevproject.h>
#include "projectmanager.h"
#include "partcontroller.h"
#include "plugincontroller.h"
#include "api.h"
#include "core.h"
#include "settingswidget.h"
#include "statusbar.h"
#include "projectsession.h"

#include "toplevel.h"
#include "mainwindowshare.h"
#include <kmditoolviewaccessor.h>

// ====================================================== class ViewMenuAction
ViewMenuAction::ViewMenuAction(
  ViewMenuActionPrivateData Data,
  const QString &Name):
  KToggleAction(Name),
  WindowData (Data)
{}

ViewMenuAction::ViewMenuAction(
  ViewMenuActionPrivateData Data,
  const QString& text, const QString& pix, const KShortcut& cut,
  const QObject* receiver, const char* slot,
  KActionCollection* parent, const char* name ):
  KToggleAction(text, pix, cut, 0,0, parent, name),
  WindowData (Data)
{
  connect(this, SIGNAL(activated(const ViewMenuActionPrivateData &)), receiver, slot);
}

void ViewMenuAction::slotActivated()
{
  KToggleAction::slotActivated();
  emit activated(WindowData);
}


/** \brief The state of a single tool window (minor helper class)
 *
 * If a tool window (type QWidget) has to be displayed, it is wrapped by a KMdiChildView
 * which in turn is embedded into a KDockWidget.
 *
 * This class works on the KDockWidget wrapper for the tool windows.
 *
 * A tool window (type KDockWidget) is embedded into the main window in 2 ways depending
 * on the number of tool windows:
 * -# If there is just one tool window, the tool window is embedded into a KDockWidget
 *    which is docked directely to the main window or which is top level, depending
 *    on the mdi mode.
 * -# If there is more than one tool window, the tool windows are put into a KDockTabWidget
 *    which is embedded into another KDockWidget which in turn is docked to the main
 *    window or which is top level, depending on the mdi mode.
 * A tool windows parent can
 * This class figures out how a single tool window is embedded. It determines the various
 * states of the tool window, its parent tab group and its dock base if those exist.
 *
 * \b Usage \n
 * if you have a KDockWidget which represents a tool window, just use
 * \code
 * KDockWidget *pDockWidget = ...
 * const ToolWindowState winState(pKDockWidget);
 * // now you can use all of winState's contents, e. g.
 * if (winState.hasDockBaseWindow) winState.pDockBaseWindow->...
 * ...
 * \endcode
 * Making winState constant makes shure nobody changes its contents after it
 * has been created. It would have also been possible to make all members private
 * and use access functions, but I don't think it is worth the trouble.
 */
class ToolWindowState
{
  public:
  ToolWindowState (KDockWidget *pDockWidget);
  bool            hasDockWidget;        //!< Is there a DockWidget (pDockWidget != 0)
  KDockTabGroup * pTabGroup;            //!< Pointer to the parent tab group or 0
  bool            hasTabGroup;          //!< true if there is a parent tab group
  bool            mayBeShown;           //!< true if there is a DockWidget and it may be shown
  bool            mayBeHide;            //!< true if there is a DockWidget and it may be hidden
  KDockWidget   * pDockBaseWindow;      //!< Pointer to the window which contains all tool windows (may be equal to pDockWidget)
  bool            hasDockBaseWindow;    //!< true, if there is a dockBaseWindow
  bool            dockBaseMayBeShow;    //!< true if thre is a dock base window and it may be shown
  bool            dockBaseMayBeHide;    //!< true if thre is a dock base window and it may be hidden
  bool            dockBaseMayBeDockBack;//!< true if thre is a dock base window and it may be dock back
  bool            dockBaseIsTopLevel;
  QString         dockBaseName;
  bool            viewMenuChecked;      //!< true if the view menu item which belongs to pDockWidget should be checked
  bool            viewMenuEnabled;      //!< true if the view menu item which belongs to pDockWidget should be enabled
  bool            dockBaseIsHidden;
  bool            dockBaseIsVisible;    //!< true if there is a dock base  and it is visible
};

ToolWindowState::ToolWindowState(KDockWidget *pDockWidget)
{
  // Determine the state of the windows inwolved
  hasDockWidget     = (pDockWidget != 0);
  pTabGroup         = (hasDockWidget)?pDockWidget->parentDockTabGroup():0L;
  hasTabGroup       = (pTabGroup != 0);
  mayBeShown        = (hasDockWidget)?pDockWidget->mayBeShow():false;
  mayBeHide         = (hasDockWidget)?pDockWidget->mayBeHide():false;

  // Search for the dock base window
  pDockBaseWindow = 0;
  if (hasTabGroup)        pDockBaseWindow = pDockWidget->dockManager()->findWidgetParentDock(pTabGroup);
  else if (hasDockWidget) pDockBaseWindow = pDockWidget->dockManager()->findWidgetParentDock(pDockWidget);
  if (mayBeHide && !pDockBaseWindow)
  {
    pDockBaseWindow = pDockWidget;  // If I'm the only tool window, I'm the dock base!
  }
  hasDockBaseWindow     = (pDockBaseWindow !=0);
  dockBaseMayBeDockBack = (hasDockBaseWindow)?pDockBaseWindow->isDockBackPossible():false;
  dockBaseMayBeShow     = (hasDockBaseWindow)?pDockBaseWindow->mayBeShow():false;
  dockBaseMayBeHide     = (hasDockBaseWindow)?pDockBaseWindow->mayBeHide():false;
  dockBaseIsTopLevel    = (hasDockBaseWindow)?(pDockBaseWindow->parent()==0):false;
  dockBaseName          = (hasDockBaseWindow)?(pDockBaseWindow->caption()):QString("");
  dockBaseIsHidden      = (hasDockBaseWindow)?(pDockBaseWindow->isHidden()):false;
  dockBaseIsVisible     = (hasDockBaseWindow)?(pDockBaseWindow->isVisible()):false;
  viewMenuChecked       = (hasTabGroup || mayBeHide)?true:false;
  viewMenuEnabled       = !dockBaseMayBeDockBack && !dockBaseMayBeShow && !dockBaseIsHidden;
}

/** \brief The state of a tool window's dock base (minor helper class)
 *
 * For a description how tool windows are embedded into the main window see ToolWindowState.
 *
 * This class takes a list of tool window , figures out who is the dock base window
 * for them and determines various state flags for the dock base.
 * Additionaly, it counts how many tool views of the list are visible (member noViews)
 *
 * \b Usage \n
 * \code
 * const ToolDockBaseState dockBaseState(m_outputViews); //MainWindow::m_outputViews
 * // now you can use all of dockBaseState's contents, e. g.
 * if (dockBaseState.hasDockBaseWindow) dockBaseState.pDockBaseWindow->...
 * ...
 * \endcode
 * Making dockBaseState constant makes shure nobody changes its contents after it
 * has been created. It would have also been possible to make all members private
 * and use access functions, but I don't think it is worth the trouble.
 */
class ToolDockBaseState
{
  public:
  ToolDockBaseState(const QPtrList<QWidget> *pViews);
  KDockWidget*      pDockBaseWindow;      //!< Pointer to the window which contains all tool windows (may be equal to pDockWidget)
  QWidget*          pFirstToolWindow;     //!< The first tool window found
  bool              hasDockBaseWindow;     //!< true if there is at least one parent tool window
  bool              dockBaseIsHidden;      //!< true if there is a parent and it is hidden
  bool              dockBaseIsVisible;     //!< true if there is a parent and it is visible
  bool              dockBaseMayBeDockBack; //!< true if thre is a parent tool window and it may be dock back
  int               noViews;               //!< number of single tool windows visible
};

ToolDockBaseState::ToolDockBaseState(const QPtrList<QWidget> *pViews):
  pDockBaseWindow      (0L),
  pFirstToolWindow     (0L),
  hasDockBaseWindow    (false),
  dockBaseIsHidden     (false),
  dockBaseIsVisible    (false),
  dockBaseMayBeDockBack(false),
  noViews              (0)
{

  return;
#warning this crashes in ideal mode, looks like there are dangling pointers on opening another project
  QPtrListIterator<QWidget> it(*pViews);
  for( ; it.current(); ++it)                                        // Iterate through all views
  {
    QWidget *pParent=it.current()->parentWidget();
    if (!pParent) continue;
    KDockWidget * pDockWidget = 0;
    pDockWidget = dynamic_cast<KDockWidget*>(pParent);
//    if (pParent->inherits("KDockWidget") || pParent->inherits("KDockWidget_Compat::KDockWidget")) pDockWidget = (KDockWidget*)pParent;
    if (!pDockWidget) continue;
    const  ToolWindowState winState(pDockWidget);
    if (!pFirstToolWindow && winState.viewMenuChecked) pFirstToolWindow = it.current();
    if (winState.hasDockBaseWindow && !hasDockBaseWindow)   // Just take the first dock base window
    {
      hasDockBaseWindow  = true;
      pDockBaseWindow    = winState.pDockBaseWindow;
      dockBaseMayBeDockBack = winState.dockBaseMayBeDockBack;
      dockBaseIsVisible     = winState.dockBaseIsVisible;
      if (winState.dockBaseIsHidden ||
         (winState.dockBaseMayBeDockBack && !winState.dockBaseIsVisible))
         dockBaseIsHidden = true;  /// @todo not just parentIsHidden = winState.parentIsHidden?

    }
    if (winState.viewMenuChecked) noViews++;
  }
}

// ====================================================== class MainWindow
MainWindow::MainWindow(QWidget *parent, const char *name, KMdi::MdiMode mdimode)
  : KMdiMainFrm(parent, name, mdimode)
  ,m_myWindowsReady(false)
  ,m_pShowOutputViews(0L)
  ,m_pShowTreeViews(0L)
  ,m_toggleViewbar(0L)
  ,m_bUiModeSwitchPending(false)
  ,m_bRemoveViewPending(false)
{
  KConfig *config = kapp->config();
  config->setGroup("UI");
  mdiStyle = config->readNumEntry("MDIStyle", 1);

  setIDEAlModeStyle(mdiStyle); // KDEV3 style of KMultiTabBar
  setStandardMDIMenuEnabled(false);
  setManagedDockPositionModeEnabled(true);

  resize( 800, 600 );
  m_pMainWindowShare = new MainWindowShare(this);

  previous_output_view = NULL;

  m_pDockbaseAreaOfDocumentViews->dockManager()->setReadDockConfigMode(KDockManager::RestoreAllDockwidgets);
}


void MainWindow::init()
{
  actionCollection()->setHighlightingEnabled( true );

#if (KDE_VERSION > 305)
  setStandardToolBarMenuEnabled( true );
#endif
  setXMLFile("kdevelopui.rc");

  createFramework();
  createActions();
  createStatusBar();

  createGUI(0);

  m_pMainWindowShare->init();

  KAction *a = actionCollection()->action("help_report_bug");
  disconnect(a, SIGNAL(activated()), 0, 0);
  connect(a, SIGNAL(activated()), m_pMainWindowShare, SLOT(slotReportBug()));

  connect(PartController::getInstance(), SIGNAL(activePartChanged(KParts::Part*)),
          this, SLOT(createGUI(KParts::Part*)));
  connect(Core::getInstance(), SIGNAL(projectOpened()),
          this, SLOT(slotReactToProjectOpened()) );
  connect(ProjectManager::getInstance()->projectSession(),
          SIGNAL(sig_restoreAdditionalViewProperties(const QString&, const QDomElement*)),
          this, SLOT(slotRestoreAdditionalViewProperties(const QString&, const QDomElement*)));
  connect(ProjectManager::getInstance()->projectSession(),
          SIGNAL(sig_saveAdditionalViewProperties(const QString&, QDomElement*)),
          this, SLOT(slotSaveAdditionalViewProperties(const QString&, QDomElement*)));

  connect(this,SIGNAL(viewActivated(KMdiChildView*)),this,SLOT(slotViewActivated(KMdiChildView*)));

  connect( PartController::getInstance(), SIGNAL(partURLChanged(KParts::ReadOnlyPart * )),
    this, SLOT(slotPartURLChanged(KParts::ReadOnlyPart * )) );

  if (!isFakingSDIApplication()) {
    QPopupMenu *menu = (QPopupMenu*) main()->child( "window", "KPopupMenu" );
    unsigned int count = menuBar()->count();
    if (menu)
        setWindowMenu(menu);
    else
        menuBar()->insertItem( i18n("&Window"), windowMenu(), -1, count-2); // standard position is left to the last ('Help')
  }

  if ( PluginController::pluginServices().isEmpty() ) {
    KMessageBox::sorry( this, i18n("Unable to find plugins, KDevelop will not work properly.\nPlease make sure "
        "that KDevelop is installed in your KDE directory, otherwise you have to add KDevelop's installation "
        "path to the environment variable KDEDIRS and run kbuildsycoca. Restart KDevelop afterwards.\n"
        "Example for BASH users:\nexport KDEDIRS=/path/to/kdevelop:$KDEDIRS && kbuildsycoca"),
        i18n("Could Not Find Plugins") );
  }
}


void MainWindow::slotViewActivated(KMdiChildView* view) {
	kdDebug()<<"void MainWindow::slotViewActivated(KMdiChildView*)"<<endl;
	QWidget *w=view->childAt(5,5);
	PartController::getInstance()->slotCurrentChanged(w);
}

MainWindow::~MainWindow()
{
  TopLevel::invalidateInstance( this );
}

bool MainWindow::queryClose()
{
    return Core::getInstance()->queryClose();
}

bool MainWindow::queryExit()
{
  saveSettings();
  return true;
}

KMainWindow *MainWindow::main()
{
  return this;
}


void MainWindow::createStatusBar()
{
  (void) new StatusBar(this);
}


void MainWindow::createFramework()
{
  PartController::createInstance(this);
  setMenuForSDIModeSysButtons(menuBar());
}

/**
 * This function adds a number of actions to the KActionCollection of the base class
 * KXMLGUIClient.
 * They are added to menus by means of the GUI-File kdevelopui.rc.
 */
void MainWindow::createActions()
{
    // Create actions for the view menu
  ViewMenuActionPrivateData ViewActionData;   // ViewActionData holds the parameter for the action
  ViewActionData.eView       = OutputView;    // The new action will be for the output tool window
  ViewActionData.pChildView  = 0L;            // It is not for a single window, but for all output tool windows
  ViewActionData.pDockWidget = 0L;            // Therefore,the window pointers are set to null
  m_pShowOutputViews = new ViewMenuAction(ViewActionData,i18n("All Output Views"), "view_bottom",
                CTRL + SHIFT + Key_O, this, SLOT(toggleToolDockBaseState(const ViewMenuActionPrivateData &)),
                actionCollection(), "output_view" );
  m_pShowOutputViews->setStatusText(i18n("Output view"));
  m_pShowOutputViews->setEnabled( true );

  ViewActionData.eView       = TreeView;      // The next action will be for the tree tool windows
  m_pShowTreeViews = new ViewMenuAction(ViewActionData,i18n("All Tree Views"), "tree_win",
                CTRL + SHIFT + Key_T, this, SLOT(toggleToolDockBaseState(const ViewMenuActionPrivateData &)),
                actionCollection(), "tree_view" );
  m_pShowTreeViews->setStatusText(i18n("Tree view"));
  m_pShowTreeViews->setEnabled( true );

  connect(manager(), SIGNAL(change()),this, SLOT(updateActionState()));

  m_pOutputToolViewsMenu = new KActionMenu( i18n("Output Tool Views"), 0, "view_output_tool_views");
  m_pOutputToolViewsMenu->setToolTip(i18n("Output tool views"));
  m_pOutputToolViewsMenu->setWhatsThis(i18n("<b>Output tool views</b><p>Shows all output views available."));
  connect(m_pOutputToolViewsMenu->popupMenu(),SIGNAL(aboutToShow()),this,SLOT(fillOutputToolViewsMenu()));
  actionCollection()->insert(m_pOutputToolViewsMenu);

  m_pTreeToolViewsMenu = new KActionMenu( i18n("Tree Tool Views"), 0, "view_tree_tool_views");
  m_pTreeToolViewsMenu->setToolTip(i18n("Tree tool views"));
  m_pTreeToolViewsMenu->setWhatsThis(i18n("<b>Tree tool views</b><p>Shows all tool views available."));
  connect(m_pTreeToolViewsMenu->popupMenu(),SIGNAL(aboutToShow()),this,SLOT(fillTreeToolViewsMenu()));
  actionCollection()->insert(m_pTreeToolViewsMenu);

  m_pMainWindowShare->createActions();
  connect(m_pMainWindowShare, SIGNAL(gotoNextWindow()), this, SLOT(gotoNextWindow()));
  connect(m_pMainWindowShare, SIGNAL(gotoPreviousWindow()), this, SLOT(gotoPreviousWindow()));
  connect(m_pMainWindowShare, SIGNAL(gotoFirstWindow()), this, SLOT(gotoFirstWindow()));
  connect(m_pMainWindowShare, SIGNAL(gotoLastWindow()), this, SLOT(gotoLastWindow()));

  m_toggleViewbar = KStdAction::showToolbar(this, SLOT(slotToggleViewbar()),actionCollection(), "settings_viewbar");
  m_toggleViewbar->setText(i18n("Show &Viewbar"));
  m_toggleViewbar->setToolTip( i18n("Show viewbar") );
  m_toggleViewbar->setWhatsThis(i18n("<b>Show viewbar</b><p>Hides or shows the viewbar."));
  showViewTaskBar(); // because start state must be equal to the action state
  if (m_mdiMode == KMdi::TabPageMode || m_mdiMode == KMdi::IDEAlMode) {
    slotToggleViewbar();
    m_toggleViewbar->setEnabled(false);
  }

  new KAction( i18n("Raise &Editor"), ALT+Key_C, this, SLOT(raiseEditor()), actionCollection(), "raise_editor");
}

KMdiChildView* MainWindow::wrapper(QWidget *view, const QString& name)
{
  QString shortName = name;
  int length = shortName.length();
  shortName = shortName.right(length - (shortName.findRev('/') +1));

  KMdiChildView* pMDICover = createWrapper(view, name, shortName);
  m_widgetMap.insert(view, pMDICover);
  m_childViewMap.insert(pMDICover, view);

  return pMDICover;
}


void MainWindow::slotPartURLChanged( KParts::ReadOnlyPart * ro_part )
{
	if ( !ro_part || !ro_part->widget() ) return;

	KMdiChildView * child = m_widgetMap[ ro_part->widget() ];
	if ( child )
	{
		child->setMDICaption( ro_part->url().fileName() );
	}
}


void MainWindow::embedPartView(QWidget *view, const QString &/*name*/, const QString& fullName)
{
  KMdiChildView *child = wrapper(view, fullName);
  m_captionDict.insert(fullName, child);

  const QPixmap* wndIcon = view->icon();
  if (!wndIcon || (wndIcon && (wndIcon->size().height() > 16))) {
    view->setIcon(SmallIcon("kdevelop"));
    child->setIcon(SmallIcon("kdevelop")); // was empty or too big, take something useful
  }


  unsigned int mdiFlags = KMdi::StandardAdd | KMdi::UseKMdiSizeHint;
  addWindow(child, mdiFlags);

  m_partViews.append(child);

  // hack for a strange geometry problem (otherwise the childview doesn't move to the right position in the childframe)
  if ((mdiMode() == KMdi::ChildframeMode) && isInMaximizedChildFrmMode()) {
    QWidget* v;
    if (child->mdiParent()) { v = child->mdiParent(); }
    else { v = child; }
    QRect geom = v->geometry();
    v->setGeometry(geom.left(),geom.top(),geom.width()+1,geom.height()+1);
    v->setGeometry(geom.left(),geom.top(),geom.width(),geom.height());
  }
}

void MainWindow::checkAndFixToolViewObjectName(QWidget* view, const QString& shortName)
{
    // a tool-view has to have an object name, otherwise it'll crash when switching the UI modes

    static unsigned int s_index4DoubledName = 0;

    QString objName = view->name();
    bool fixed = false;
    if (objName.isEmpty()) {
        objName = shortName;
        fixed = true;
    }
    if (objName == "unnamed") {
        objName = shortName;
        fixed = true;
    }
    if (objName.isEmpty()) {
        QString s; s.setNum(s_index4DoubledName++);
        objName += "unnamed_" + s;
        fixed = true;
    }
    if (fixed) {
        view->setName(objName.latin1());
    }
}

void MainWindow::embedSelectView(QWidget *view, const QString &name, const QString &toolTip)
{
    const QPixmap* wndIcon = view->icon();
    kdDebug(9000) << "icon = " << wndIcon << endl;
    if (!wndIcon ) { // || (wndIcon && (wndIcon->size().height() > 16))) {
        view->setIcon(SmallIcon("kdevelop")); // was empty or too big, take something useful
    }

    QString shortName = name;
    int length = shortName.length();
    shortName = shortName.right(length - (shortName.findRev('/') +1)).stripWhiteSpace();
    checkAndFixToolViewObjectName(view, shortName);

    KMdiToolViewAccessor *tmp =
        KMdiMainFrm::addToolWindow(view,
                                   KDockWidget::DockLeft,
                                   getMainDockWidget(),
                                   25,
                                   toolTip,
                                   mdiMode() == KMdi::IDEAlMode ? shortName : "" // empty tab caption, only icons - otherwise it'll be too wide!
                                  );

    m_selectViews.append(view);
}

void MainWindow::embedSelectViewRight(QWidget* view, const QString& title, const QString &toolTip)
{
    if (mdiMode() != KMdi::IDEAlMode) {
        //we don't like a right pane in several UI modes, dock to default
        embedSelectView( view, title, toolTip );
        return;
    }


    const QPixmap* wndIcon = view->icon();
    kdDebug(9000) << "icon = " << wndIcon << endl;
    if (!wndIcon ) { // || (wndIcon && (wndIcon->size().height() > 16))) {
        view->setIcon(SmallIcon("kdevelop")); // was empty or too big, take something useful
    }

    QString shortName = title;
    int length = shortName.length();
    shortName = shortName.right(length - (shortName.findRev('/') +1)).stripWhiteSpace();
    checkAndFixToolViewObjectName(view, shortName);

    KMdiToolViewAccessor *tmp=KMdiMainFrm::addToolWindow(view,
                                                         KDockWidget::DockRight,
                                                         getMainDockWidget(),
                                                         25,
                                                         toolTip,
                                                         shortName);
    m_selectViews.append(view);
}

void MainWindow::embedOutputView(QWidget *view, const QString &name, const QString &toolTip)
{
    QString shortName = name;
    int length = shortName.length();
    shortName = shortName.right(length - (shortName.findRev('/') +1)).stripWhiteSpace();
    checkAndFixToolViewObjectName(view, shortName);

    KMdiToolViewAccessor *tmp =
        KMdiMainFrm::addToolWindow(view,
                                   KDockWidget::DockBottom,
                                   getMainDockWidget(),
                                   70,
                                   toolTip,
                                   shortName
                                   );

    m_outputViews.append(view);
}


void MainWindow::setViewAvailable(QWidget *pView, bool bEnabled)
{
  if (!pView) return;

  KDockWidget* pWrappingDockWidget = dockManager->findWidgetParentDock(pView);
  if (!pWrappingDockWidget) return;
  if (bEnabled) {
    pWrappingDockWidget->makeDockVisible();
  }
  else {
    pWrappingDockWidget->undock();
  }

  if (bEnabled) {
    m_unavailableViews.remove(pView);
  }
  else {
    m_unavailableViews.append(pView);
  }
}

void MainWindow::removeView(QWidget *view)
{
  if (!view) {
    return;
  }
  m_bRemoveViewPending = true;

  QGuardedPtr<KMdiChildView> wrapper = m_widgetMap[view];

  if (wrapper) {
    removeWindowFromMdi(wrapper);
    m_partViews.remove(wrapper);
    m_widgetMap.remove(view);
    m_childViewMap.remove(wrapper);
    m_captionDict.remove(wrapper->caption());
  }

  // Find the KDockWidget which covers the KMdiChildView to remove and delete.
  // Undock the KDockWidget if there is one.
  // This will remove the corresponding tab from the output and tree views.
  QGuardedPtr<KDockWidget> pDock = dockManager->findWidgetParentDock(wrapper ? wrapper : view);

  // Note: this reparenting is necessary. Otherwise, the view gets
  // deleted twice: once when the wrapper is deleted, and the second
  // time when the part is deleted.
  view->hide();
  view->reparent(this, QPoint(0,0), false);

  // QextMDI removes and deletes the wrapper widget
  // removed by robe.. seems that this fix the crash when exit from kdevelop in MDI mode
  if (wrapper) {
    closeWindow(wrapper);
  }

  if( pDock ) {
	deleteToolWindow(view);
/*    pDock->undock();
    delete (KDockWidget*) pDock;*/
  }
  else {
    kdDebug(9000) << "pDock already destroyed!!" << endl;
  }
  if (!wrapper) {
    m_selectViews.remove(view);
    m_outputViews.remove(view);
  }

  m_bRemoveViewPending = false;
}


void MainWindow::raiseView(QWidget *view)
{
    m_myWindowsReady = true;                                    // From now on, we can rely on the windows beeing active
    // Handle differences between output and tree views
    QPtrList<QWidget> *pViews        = 0L;             // The views to make a menu from

    QWidget *wrapper = m_widgetMap[ view ];
    if (wrapper)
        wrapper->setFocus();

    KDockWidget* pDock = dockManager->findWidgetParentDock(wrapper ? wrapper : view);
    if (pDock)
        if (!pDock->isVisible())
            makeDockVisible(pDock);
}


void MainWindow::lowerView(QWidget *)
{
  // ignored in MDI mode!
}

void MainWindow::lowerAllViews()
{
    // ignored in MDI mode!
}


void MainWindow::createGUI(KParts::Part *part)
{
  if ( !part )
    setCaption( QString::null );
  KMdiMainFrm::createGUI(part);

  m_pMainWindowShare->slotGUICreated( part );
}


void MainWindow::loadSettings()
{
  ProjectManager::getInstance()->loadSettings();
  loadMDISettings();
  applyMainWindowSettings(kapp->config(), "Mainwindow");
}


void MainWindow::loadMDISettings()
{
  KConfig *config = kapp->config();
  config->setGroup("UI");

  int mdiMode = config->readNumEntry("MDIMode", KMdi::IDEAlMode);
  switch (mdiMode)
  {
  case KMdi::ToplevelMode:
    {
      int childFrmModeHt = config->readNumEntry("Childframe mode height", kapp->desktop()->height() - 50);
      resize(width(), childFrmModeHt);
      switchToToplevelMode();
    }
    break;

  case KMdi::ChildframeMode:
    break;

  case KMdi::TabPageMode:
    {
      int childFrmModeHt = config->readNumEntry("Childframe mode height", kapp->desktop()->height() - 50);
      resize(width(), childFrmModeHt);
      switchToTabPageMode();
    }
    break;
  case KMdi::IDEAlMode:
    {
      switchToIDEAlMode();
    }

  default:
    break;
  }

  // restore a possible maximized Childframe mode
  bool maxChildFrmMode = config->readBoolEntry("maximized childframes", true);
  setEnableMaximizedChildFrmMode(maxChildFrmMode);

  readDockConfig(0L, "dockSession_version1");
}

void MainWindow::guiRestoringFinished()
{
   dockManager->finishReadDockConfig();
}

void MainWindow::prepareToCloseViews()
{
  writeDockConfig(0L, "dockSession_version1");
}

void MainWindow::saveSettings()
{
  ProjectManager::getInstance()->saveSettings();
  saveMainWindowSettings(kapp->config(), "Mainwindow");
  saveMDISettings();
}


void MainWindow::saveMDISettings()
{
  KConfig *config = kapp->config();
  config->setGroup("UI");

  config->writeEntry("maximized childframes", isInMaximizedChildFrmMode());
  config->writeEntry("MDIStyle", mdiStyle);
  config->writeEntry("MDIMode", mdiMode());
}


void MainWindow::resizeEvent(QResizeEvent *ev)
{
  KMdiMainFrm::resizeEvent(ev);
  setSysButtonsAtMenuPosition();
}

void MainWindow::childWindowCloseRequest(KMdiChildView *pWnd)
{
  PartController::getInstance()->closePartForWidget( m_childViewMap[pWnd] );
}

void MainWindow::gotoNextWindow()
{
  activateNextWin();
}

void MainWindow::gotoPreviousWindow()
{
  activatePrevWin();
}

void MainWindow::gotoFirstWindow()
{
  activateFirstWin();
}

void MainWindow::gotoLastWindow()
{
  activateLastWin();
}

//=============== fillWindowMenu ===============//
// This is more or less a verbatim copy from what is implemented in KMdiMainFrm
void MainWindow::fillWindowMenu()
{
   bool bTabPageMode = FALSE;
   if (m_mdiMode == KMdi::TabPageMode)
      bTabPageMode = TRUE;
   bool bIDEAlMode = FALSE;
   if (m_mdiMode == KMdi::IDEAlMode)
      bIDEAlMode = TRUE;

   bool bNoViewOpened = FALSE;
   if (m_pDocumentViews->isEmpty()) {
      bNoViewOpened = TRUE;
   }
   // construct the menu and its submenus
   if (!m_bClearingOfWindowMenuBlocked) {
      clearWindowMenu();
   }
   int closeId         = m_pWindowMenu->insertItem(i18n("&Close"), PartController::getInstance(), SLOT(slotCloseWindow()));
   m_windowMenus.append(closeId);
   int closeAllId      = m_pWindowMenu->insertItem(i18n("Close &All"), PartController::getInstance(), SLOT(slotCloseAllWindows()));
   m_windowMenus.append(closeAllId);
   int closeAllOtherId = m_pWindowMenu->insertItem(i18n("Close All &Others"), PartController::getInstance(), SLOT(slotCloseOtherWindows()));
   m_windowMenus.append(closeAllOtherId);
   if (bNoViewOpened) {
      m_pWindowMenu->setItemEnabled(closeId, FALSE);
      m_pWindowMenu->setItemEnabled(closeAllId, FALSE);
      m_pWindowMenu->setItemEnabled(closeAllOtherId, FALSE);
   }
   if (!bTabPageMode && !bIDEAlMode) {
      int iconifyId = m_pWindowMenu->insertItem(i18n("&Minimize All"), this, SLOT(iconifyAllViews()));
      m_windowMenus.append(iconifyId);
      if (bNoViewOpened) {
         m_pWindowMenu->setItemEnabled(iconifyId, FALSE);
      }
   }
   m_windowMenus.append(m_pWindowMenu->insertSeparator());
   if (!bTabPageMode && !bIDEAlMode) {
      int placMenuId = m_pWindowMenu->insertItem(i18n("&Tile..."), m_pPlacingMenu);
      m_windowMenus.append(placMenuId);
      m_pPlacingMenu->clear();
      m_pPlacingMenu->insertItem(i18n("Ca&scade Windows"), m_pMdi,SLOT(cascadeWindows()));
      m_pPlacingMenu->insertItem(i18n("Cascade &Maximized"), m_pMdi,SLOT(cascadeMaximized()));
      m_pPlacingMenu->insertItem(i18n("Expand &Vertically"), m_pMdi,SLOT(expandVertical()));
      m_pPlacingMenu->insertItem(i18n("Expand &Horizontally"), m_pMdi,SLOT(expandHorizontal()));
      m_pPlacingMenu->insertItem(i18n("Tile &Non-overlapped"), m_pMdi,SLOT(tileAnodine()));
      m_pPlacingMenu->insertItem(i18n("Tile Overla&pped"), m_pMdi,SLOT(tilePragma()));
      m_pPlacingMenu->insertItem(i18n("Tile V&ertically"), m_pMdi,SLOT(tileVertically()));
      if (m_mdiMode == KMdi::ToplevelMode) {
         m_pWindowMenu->setItemEnabled(placMenuId, FALSE);
      }
      m_windowMenus.append(m_pWindowMenu->insertSeparator());
      int dockUndockId = m_pWindowMenu->insertItem(i18n("&Dock/Undock..."), m_pDockMenu);
      m_windowMenus.append(dockUndockId);
      m_pDockMenu->clear();
      m_windowMenus.append(m_pWindowMenu->insertSeparator());
      if (bNoViewOpened) {
         m_pWindowMenu->setItemEnabled(placMenuId, FALSE);
         m_pWindowMenu->setItemEnabled(dockUndockId, FALSE);
      }
   }
   int entryCount = m_pWindowMenu->count();

   // for all child frame windows: give an ID to every window and connect them in the end with windowMenuItemActivated()
   int i=100;
   KMdiChildView* pView = 0L;
   QPtrListIterator<KMdiChildView> it(*m_pDocumentViews);
   QValueList<QDateTime> timeStamps;
   for (; it.current(); ++it) {

      pView = it.current();
      if (pView->isToolView()) {
         continue;
      }
      QDateTime timeStamp( pView->getTimeStamp() );

      KParts::ReadOnlyPart * ro_part = getPartFromWidget(m_childViewMap[pView]);

      QString name = (ro_part==0L)?pView->caption():ro_part->url().prettyURL();
      QString item;
      // set titles of minimized windows in brackets
      if (pView->isMinimized()) {
         item += "(";
         item += name;
         item += ")";
      }
      else {
         item += " ";
         item += name;
       }

      // insert the window entry sorted in alphabetical order
      unsigned int indx;
      unsigned int windowItemCount = m_pWindowMenu->count() - entryCount;
//      bool inserted = FALSE;
      QString tmpString;

      QValueList<QDateTime>::iterator timeStampIterator = timeStamps.begin();
      for (indx = 0; indx < windowItemCount; indx++, ++timeStampIterator) {
        bool putHere = false;
        if ((*timeStampIterator) < timeStamp) {
          putHere = true;
        }
        if ( putHere ) {
//          timeStamps.insert( timeStampIterator, timeStamp );
//          m_pWindowMenu->insertItem( item, pView, SLOT(slot_clickedInWindowMenu()), 0, -1, indx+entryCount);
//           if (pView == m_pCurrentWindow)
//             m_pWindowMenu->setItemChecked( m_pWindowMenu->idAt( indx+entryCount), TRUE);
//           pView->setWindowMenuID( i);
//           if (!bTabPageMode) {
//             m_pDockMenu->insertItem( item, pView, SLOT(slot_clickedInDockMenu()), 0, -1, indx);
//             if (pView->isAttached())
//                 m_pDockMenu->setItemChecked( m_pDockMenu->idAt( indx), TRUE);
//           }
//           inserted = TRUE;
          break;
        }
      }
//      if (!inserted) {  // append it
         timeStamps.insert( timeStampIterator, timeStamp );
            m_windowMenus.append(m_pWindowMenu->insertItem( item, pView, SLOT(slot_clickedInWindowMenu()), 0, -1, indx+entryCount));
            if (pView == m_pCurrentWindow)
               m_pWindowMenu->setItemChecked( m_pWindowMenu->idAt( indx+entryCount), TRUE);
            pView->setWindowMenuID( i);
            if (!bTabPageMode) {
               m_windowMenus.append(m_pDockMenu->insertItem( item, pView, SLOT(slot_clickedInDockMenu()), 0, -1, indx));
               if (pView->isAttached())
                  m_pDockMenu->setItemChecked( m_pDockMenu->idAt( indx), TRUE);
            }
//      }
      i++;
   }
}

/** Fills the show-hide menu for the output views */
void MainWindow::fillOutputToolViewsMenu()
{
  fillToolViewsMenu(OutputView);    // Fill tool-view menu for output views
}

/** Fills the show-hide menu for the tree views */
void MainWindow::fillTreeToolViewsMenu()
{
  fillToolViewsMenu(TreeView);      // Fill tool-view menu for tree views
}

//=============== fillToolViewsMenu ===============//
/** Fills the show-hide menu for a tool view (output or tree view)
 *
 * The menu has the following entries:
 * - An item to show or hide the dock base window ("All Tree Views" or " All Ouput Views")
 * - A seperator
 * - An item for each tool window. The item is disabled, if the dock base is not visible.
 *   It is checked, if the tool window would be visible or has a tab page if the dock base
 *   is made visible
 */
void MainWindow::fillToolViewsMenu(
     EView eView)
{
  // Handle differences between output and tree views
  QPtrList<QWidget>* pViews          = 0L;         // The views to make a menu from
  KActionMenu*       pActionMenu     = 0L;         // The menu to build
  ViewMenuAction*    pAllViewsAction = 0L;         // Pointer to action which will toggle the state of all windows

  if (eView == OutputView)
  {
    pViews          = &m_outputViews;
    pActionMenu     = m_pOutputToolViewsMenu;
    pAllViewsAction = m_pShowOutputViews;
  }
  else
  {
    pViews          = &m_selectViews;
    pActionMenu     = m_pTreeToolViewsMenu;
    pAllViewsAction = m_pShowTreeViews;
  }

  ToolDockBaseState allToolWinState(pViews);

  // Prepare fixed part of the menu
  pActionMenu->popupMenu()->clear();                                // Remove all entries
  pActionMenu->insert(pAllViewsAction);
  pActionMenu->popupMenu()->insertSeparator();

  // Prepare variable part of the menu
  m_myWindowsReady = true;                                            // From now on, we can rely on the windows beeing active
  QPtrListIterator<QWidget> it(*pViews);
  for( ; it.current(); ++it)                                          // Iterate through all views
  {
     if (m_unavailableViews.find(it.current()) != -1) {
        continue; // skip this view, it was set to unavailable for the GUI
     }
     KDockWidget *pDockWidget = manager()->findWidgetParentDock(it.current());  // Get the DockWidget which covers the view
     Q_ASSERT(pDockWidget);
     QString Name = pDockWidget->tabPageLabel();                    // Get the name of the view
     ViewMenuActionPrivateData ActionData;
     ActionData.pDockWidget = pDockWidget;                          // Save the pointer to the DockWidget
     ActionData.pChildView = it.current();                          // Save the pointer to the view
     ActionData.eView      = eView;                                 // Save whether it is an output or tree view

     ViewMenuAction* action = new ViewMenuAction(ActionData,Name);  // Action to show or hide the view window
     connect(action,                                                // Call toggleSingleToolWin if the action is activated
             SIGNAL(activated(const ViewMenuActionPrivateData &)),
             this,
             SLOT(toggleSingleToolWin(const ViewMenuActionPrivateData &)));

     const  ToolWindowState winState(pDockWidget);
     action->setChecked(winState.viewMenuChecked);
     action->setEnabled(
             (allToolWinState.hasDockBaseWindow && !allToolWinState.dockBaseIsHidden)
             || (allToolWinState.noViews==0));
     pActionMenu->insert(action);
  }
}

/** Updates the toggle state of the actions to show or hide the tool windows */
void MainWindow::updateActionState()
{
    if (!m_bUiModeSwitchPending && !m_bRemoveViewPending) {
        ToolDockBaseState outputToolState(&m_outputViews);
        ToolDockBaseState treeToolState (&m_selectViews);

        m_pShowOutputViews->setChecked(outputToolState.dockBaseIsVisible);
        m_pShowTreeViews  ->setChecked(treeToolState.dockBaseIsVisible);
    }
}

/** Changes the show-hide state of a tool dock base (either output or tree tool view)*/
void MainWindow::toggleToolDockBaseState(const ViewMenuActionPrivateData &ActionData)
{
  m_myWindowsReady = true;                    // From now on, we can rely on the windows beeing active
  // Handle differences between output and tree views
  QPtrList<QWidget> *pViews = 0L;             // The views to make a menu from
  pViews = (ActionData.eView == OutputView)?&m_outputViews:&m_selectViews;

  QPtrListIterator<QWidget> it(*pViews);
  const  ToolDockBaseState allWinState(pViews);

  if(allWinState.dockBaseIsVisible) {         // If it is visible
    allWinState.pDockBaseWindow->undock();    // undock it, so it is invisible
  }
  else if (allWinState.dockBaseMayBeDockBack){// If it may be dock back, it is invisible
    allWinState.pDockBaseWindow->dockBack();  // Show it again
  }
  else if(allWinState.dockBaseIsHidden) {     // In toplevel mode the tool window is just hidden
    allWinState.pDockBaseWindow->show();      // Then show it
  }
  else
  {
    // not a single tool window found, so we show all of them
    showAllToolWin(ActionData.eView,1);
  }

}

/** Shows all tools views of a type (OutputView or TreeView*/
void MainWindow::showAllToolWin(EView eView, bool show )
{
  // Handle differences between output and tree views
  QPtrList<QWidget> *pViews = 0L;         // The views to make a menu from

  pViews =(eView==OutputView)?&m_outputViews:&m_selectViews;

  // If the tool window is not visible, first show it!
  const  ToolDockBaseState allWinState(pViews);
  if(allWinState.dockBaseIsHidden)
  {
    ViewMenuActionPrivateData ActionData;
    ActionData.eView = eView;
    ActionData.pChildView = 0;
    ActionData.pDockWidget = 0;
    toggleToolDockBaseState(ActionData);
  }

  // Now switch on every single tool window
  m_myWindowsReady = true;                                            // From now on, we can rely on the windows beeing active
  QPtrListIterator<QWidget> it(*pViews);
  for( ; it.current(); ++it)                                          // Iterate through all views
  {
     if (m_unavailableViews.find(it.current()) != -1) {
	 continue;
     }
     KDockWidget *pDockWidget=manager()->findWidgetParentDock(it.current());  // Get the DockWidget which covers the view
     Q_ASSERT(pDockWidget);
     QString Name = pDockWidget->name();                            // Get the name of the view
     ViewMenuActionPrivateData ActionData;
     ActionData.pDockWidget = pDockWidget;                          // Save the pointer to the DockWidget
     ActionData.pChildView = it.current();                          // Save the pointer to the view
     ActionData.eView      = eView;                                 // Save whether it is an output or tree view
     const  ToolWindowState winState(pDockWidget);
     if(winState.viewMenuChecked != show)
             toggleSingleToolWin(ActionData);
  }

}

/** Changes the show-hide state of a single tree or output tool window */
void MainWindow::toggleSingleToolWin(const ViewMenuActionPrivateData &ActionData)
{
  // Determine the state of the windows inwolved
  const  ToolWindowState winState(ActionData.pDockWidget);
  QPtrList<QWidget> *pViews = 0L;             // The views to make a menu from
  pViews = (ActionData.eView == OutputView)?&m_outputViews:&m_selectViews;

  const  ToolDockBaseState allWinState(pViews);
  if(winState.hasTabGroup)
  {                                                    // the window has a tab page
    if (winState.mayBeShown)
    {
      QWidget *pActiveWidget=winState.pTabGroup->currentPage(); // remember the active page
      ActionData.pDockWidget->changeHideShowState();   // pDockWidget has not not been the active page,now it is acitve
      ActionData.pDockWidget->changeHideShowState();   // and now it is gone
      if (allWinState.noViews > 2)                     // Is still more than one page left?
      {                                                // Yes, so pTabGroup still exists...
        winState.pTabGroup->showPage(pActiveWidget);              // switch back to previouse active page
      }
    }
    else
    {
      ActionData.pDockWidget->changeHideShowState();   // and now it is gone
    }
  }
  else    // It does not have a tab group
  {
    if (winState.hasDockWidget)
    {
      if(winState.mayBeHide)
      {
         ActionData.pDockWidget->changeHideShowState();   // and now it is gone
      }
      else
      {
        // Count how many windows are visible
        KDockWidget *first =0L;   // Pointer to a widget in that view area, may function as target to docking
        QPtrListIterator<QWidget> it((ActionData.eView==OutputView)?m_outputViews:m_selectViews);
        for( ; it.current(); ++it)                              // Iterate through all  views
        {
          KDockWidget *pDockWidget=manager()->findWidgetParentDock(it.current());  // Get the DockWidget which covers the view
          if(pDockWidget)
          {
            KDockTabGroup *pTabGroup = pDockWidget->parentDockTabGroup(); //  Get the TabGroup which belongs to pDockWidget
            if (pDockWidget->mayBeHide() || pTabGroup)                    // The window exists, if it has a TabGroup or if it can be hidden
            {
              first = manager()->findWidgetParentDock(it.current());  // Get the DockWidget which covers the view
              break;
            }
          }
        }
        if (!first)
        {
          if (mdiMode() == KMdi::TabPageMode)
          {
            first = manager()->findWidgetParentDock (m_partViews.first());
          }
          if (!first) first = m_pDockbaseAreaOfDocumentViews;
          if (ActionData.eView == OutputView) ActionData.pDockWidget->manualDock(first,KDockWidget::DockBottom, 70);
          else                                ActionData.pDockWidget->manualDock(first,KDockWidget::DockLeft  , 25);
        }
        else
        {
          ActionData.pDockWidget->manualDock(first,KDockWidget::DockCenter, 25);
        }
      }
    }
    else // It does not have a DockWidget
    {
      Q_ASSERT(0);
#if 0
         addToolViewWindow(ActionData.eView, ActionData.pChildView, ActionData.pChildView->name(), ActionData.pChildView->name());
#endif
    }
  }
}


//=============== getPartFromWidget ===============//
KParts::ReadOnlyPart * MainWindow::getPartFromWidget(const QWidget * pWidget) const
{
  // Loop over all parts to search for a matching widget
  QPtrListIterator<KParts::Part> it(*(PartController::getInstance()->parts()));
  for ( ; it.current(); ++it)
  {
    KParts::ReadOnlyPart *ro_part = dynamic_cast<KParts::ReadOnlyPart*>(it.current());
    if (ro_part->widget() == pWidget) return ro_part;
  }
  return (0L);
}

void MainWindow::switchToToplevelMode()
{
  if (m_mdiMode == KMdi::ToplevelMode) return;
  m_bUiModeSwitchPending = true;

  m_toggleViewbar->setEnabled(true);
  if (mdiMode() == KMdi::TabPageMode || mdiMode() == KMdi::IDEAlMode) {
      slotToggleViewbar();
  }
  KMdiMainFrm::switchToToplevelMode();

  m_bUiModeSwitchPending = false;
  updateActionState();
  saveMDISettings();
}

void MainWindow::switchToChildframeMode()
{
  if (m_mdiMode == KMdi::ChildframeMode) return;
  m_bUiModeSwitchPending = true;

  m_toggleViewbar->setEnabled(true);
  if (mdiMode() == KMdi::TabPageMode || mdiMode() == KMdi::IDEAlMode) {
      slotToggleViewbar();
  }
  KMdiMainFrm::switchToChildframeMode();

  m_bUiModeSwitchPending = false;
  updateActionState();
  saveMDISettings();
}

void MainWindow::switchToTabPageMode()
{
  if (m_mdiMode == KMdi::TabPageMode) return;
  m_bUiModeSwitchPending = true;

  if (isViewTaskBarOn()) {
      slotToggleViewbar();
  }
  m_toggleViewbar->setEnabled(false);
  KMdiMainFrm::switchToTabPageMode();

  m_bUiModeSwitchPending = false;
  updateActionState();
  saveMDISettings();
}

void MainWindow::switchToIDEAlMode()
{
  if (m_mdiMode == KMdi::IDEAlMode) return;
  m_bUiModeSwitchPending = true;

  if (isViewTaskBarOn()) {
      slotToggleViewbar();
  }
  m_toggleViewbar->setEnabled(false);
  KMdiMainFrm::switchToIDEAlMode();

  m_bUiModeSwitchPending = false;
  updateActionState();
  saveMDISettings();
}

void MainWindow::slotReactToProjectOpened()
{
    // This is a workaround for a bug in KDockWidget::readDockConfig() called above:
    // We must hide the unavailable views again because they are somehow shown again here
    // (unfortunately, we can't avoid the flickering which is a result of that show()-hide() calling)
    QPtrListIterator<QWidget> it(m_unavailableViews);
    for (; it.current(); ++it) {
	KDockWidget* pWrappingDockWidget = dockManager->findWidgetParentDock(*it);
	if (pWrappingDockWidget) {
	    pWrappingDockWidget->hide();
	    pWrappingDockWidget->undock();
	}
    }
}

void MainWindow::slotRestoreAdditionalViewProperties(const QString& viewName, const QDomElement* viewEl)
{
  KMdiChildView* pMDICover = m_captionDict[viewName];
  if (!pMDICover) { return; }

  // read the view position and size
  int nMinMaxMode = viewEl->attribute( "MinMaxMode", "0").toInt();
  int   nLeft     = viewEl->attribute( "Left", "-10000").toInt(); // XXX hack: value -10000 wouldn't be restored correctly
  int   nTop      = viewEl->attribute( "Top", "-10000").toInt();
  int   nWidth    = viewEl->attribute( "Width", "-1").toInt();
  int   nHeight   = viewEl->attribute( "Height", "-1").toInt();

  // MDI stuff
  bool bAttached = (bool) viewEl->attribute( "Attach", "1").toInt();

  // restore appearence
  if ((mdiMode() != KMdi::TabPageMode) && (mdiMode() != KMdi::ToplevelMode) && (mdiMode() != KMdi::IDEAlMode)) {
    if ((!pMDICover->isAttached()) && (bAttached) ) {
      pMDICover->attach();
    }
    if ( (pMDICover->isAttached()) && (!bAttached) ) {
      pMDICover->detach();
    }
  }
  if (nMinMaxMode == 0) {
    pMDICover->setInternalGeometry(QRect(nLeft, nTop, nWidth, nHeight));
  }
  else {
    if (nMinMaxMode == 1) {
      pMDICover->minimize();
    }
    if (nMinMaxMode == 2) {
      // maximize: nothing to do, this is already under control of the mainframe
    }
    pMDICover->setRestoreGeometry(QRect(nLeft, nTop, nWidth, nHeight));
  }
}

void MainWindow::slotSaveAdditionalViewProperties(const QString& viewName, QDomElement* viewEl)
{
  KMdiChildView* pMDICover = m_captionDict[viewName];
  if (!pMDICover) { return; }

  // write the view position and size
  QRect geom;
  int nMinMaxMode = 0;
  if (pMDICover->isMinimized()) {
    nMinMaxMode = 1;
  }
  if (pMDICover->isMaximized()) {
    nMinMaxMode = 2;
  }
  if (nMinMaxMode == 0) {
    geom = pMDICover->internalGeometry();
  }
  else {
    geom = pMDICover->restoreGeometry();
  }
  viewEl->setAttribute( "MinMaxMode", nMinMaxMode);
  viewEl->setAttribute( "Left", geom.left());
  viewEl->setAttribute( "Top", geom.top());
  viewEl->setAttribute( "Width", geom.width());
  viewEl->setAttribute( "Height", geom.height());

  // MDI stuff
  viewEl->setAttribute( "Attach", pMDICover->isAttached() || (mdiMode() == KMdi::TabPageMode) || (mdiMode() == KMdi::IDEAlMode));
}

void MainWindow::slotToggleViewbar()
{
    slot_toggleTaskBar();
}

void MainWindow::setUserInterfaceMode(const QString& uiMode)
{
    // immediately switch the mode likely set in the uimode part
    if (uiMode == "Childframe") {
	switchToChildframeMode();
    }
    else if (uiMode == "TabPage") {
	switchToTabPageMode();
    }
    else if (uiMode == "Toplevel") {
	switchToToplevelMode();
    }
    else if (uiMode == "KMDI-IDEAl") {
        switchToIDEAlMode();
    }
}

void MainWindow::callCommand(const QString& command)
{
    if (isInMaximizedChildFrmMode() && (command == "qextmdi-UI: do hack on session loading finished")
    && (mdiMode() == KMdi::ChildframeMode)) {
	KMdiChildView* pLastView = 0L;
	KMdiChildFrm*  pLastFrm = 0L;
	KMdiIterator<KMdiChildView*>* winListIter = createIterator();
	for (winListIter->first(); !winListIter->isDone(); winListIter->next()){
	    pLastView = winListIter->currentItem();
	    if (pLastView->isAttached()) {
		pLastFrm = pLastView->mdiParent();
	    }
	}
	// evil hack (of Falk): resize the childframe again 'cause of event timing probs with resizing
	if (pLastFrm && pLastFrm->parentWidget()) {
	    QApplication::sendPostedEvents();
	    pLastFrm->setGeometry(
		-KMDI_CHILDFRM_BORDER,
		-KMDI_CHILDFRM_BORDER - pLastFrm->captionHeight() - KMDI_CHILDFRM_SEPARATOR,
		pLastFrm->parentWidget()->width() + KMDI_CHILDFRM_DOUBLE_BORDER,
		pLastFrm->parentWidget()->height() + KMDI_CHILDFRM_SEPARATOR
		+ KMDI_CHILDFRM_DOUBLE_BORDER + pLastFrm->captionHeight());
	}
	delete winListIter;
    }
}

/**
 * Store the currently active view tab/window of the output view/window
 */
void MainWindow::storeOutputViewTab()
{
/** @todo implement this properly
       look at MainWindow::slotReactToProjectOpened() to hide a view
       MainWindow::toggleSingleToolWin() to show it and also to check the visibility

  if (m_bottomBar->isRaised()) {
    // If m_bottomBar->isRaised then store the current view
    previous_output_view = m_bottomBar->current();
  }else {
    // If not hide the bottom view by storing NULL
    previous_output_view = NULL;
  }
*/
}


/**
 * Restore the previously saved view tab/window to the output view/window
 */
void MainWindow::restoreOutputViewTab()
{
/** @todo implement this properly
       look at MainWindow::slotReactToProjectOpened() to hide a view
       MainWindow::toggleSingleToolWin() to show it and also to check the visibility

  if ( previous_output_view == NULL) {
    // If no previous exists then hide current
    if (m_bottomBar->current())
      lowerView(m_bottomBar->current());
  } else {
    // previous exist, so raise it
    m_bottomBar->raiseWidget(previous_output_view);
  }
  previous_output_view = NULL;
*/
}

void MainWindow::clearWindowMenu( )
{
    for (QValueList<int>::iterator it = m_windowMenus.begin(); it != m_windowMenus.end(); ++it)
    {
        m_pWindowMenu->removeItem(*it);
    }
}

void MainWindow::setWindowMenu(QPopupMenu *menu)
{
    if (m_pWindowMenu)
        delete m_pWindowMenu;
    m_pWindowMenu = menu;
    m_pWindowMenu->setCheckable( TRUE);
    QObject::connect( m_pWindowMenu, SIGNAL(aboutToShow()), this, SLOT(fillWindowMenu()) );
}

void MainWindow::setCaption( const QString & caption )
{
	KDevProject * project = API::getInstance()->project();
	if ( project && !caption.isEmpty() )
	{
		QString projectname = project->projectName();

		QString suffix(".kdevelop");
		if ( projectname.endsWith( suffix ) )
		{
			projectname.truncate( projectname.length() - suffix.length() );
		}

		KMdiMainFrm::setCaption( projectname + " - " + caption );
	}
	else
	{
		KMdiMainFrm::setCaption( caption );
	}
}

void MainWindow::raiseEditor( )
{
	kdDebug() << k_funcinfo << endl;
	
	KDevPartController * partcontroller = API::getInstance()->partController();
	if ( partcontroller->activePart() && partcontroller->activePart()->widget() )
	{
		partcontroller->activePart()->widget()->setFocus();
	}
}

#include "mainwindow.moc"
