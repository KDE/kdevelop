//----------------------------------------------------------------------------
//    filename             : kmdimainfrm.cpp
//----------------------------------------------------------------------------
//    Project              : KDE MDI extension
//
//    begin                : 07/1999       by Szymon Stefanek as part of kvirc
//                                         (an IRC application)
//    changes              : 09/1999       by Falk Brettschneider to create an
//                           - 06/2000     stand-alone Qt extension set of
//                                         classes and a Qt-based library
//                           2000-2003     maintained by the KDevelop project
//    patches              : 02/2000       by Massimo Morin (mmorin@schedsys.com)
//                           */2000        by Lars Beikirch (Lars.Beikirch@gmx.net)
//                           01/2003       by Jens Zurheide (jens.zurheide@gmx.de)
//
//    copyright            : (C) 1999-2003 by Szymon Stefanek (stefanek@tin.it)
//                                         and
//                                         Falk Brettschneider
//    email                :  falkbr@kdevelop.org (Falk Brettschneider)
//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU Library General Public License as
//    published by the Free Software Foundation; either version 2 of the
//    License, or (at your option) any later version.
//
//----------------------------------------------------------------------------


/*
 * ATTENTION: please do you part to try to make this file legible.  It's
 * extremely hard to read already.  Especially follow the indenting rules.
 */
#include "config.h"

#include <assert.h>

#include <qcursor.h>
#include <qclipboard.h>
#include <qobjectlist.h>
#include <qpopupmenu.h>
#include <qmenubar.h>

#include <kmenubar.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kdeversion.h>
#include <qtabwidget.h>
#include <klocale.h>
#if defined Q_WS_X11 && ! defined K_WS_QTONLY
#include <kipc.h> // schroder remove this in x11 too, not needed any more...
#endif

#include <kiconloader.h>
#include "kmdidockcontainer.h"


#include <qtoolbutton.h>
#include <qlayout.h>
#include <qtimer.h>
#include <qtextstream.h>
#include <qstring.h>
#include <qmap.h>
#include <qvaluelist.h>

#include "kmdimainfrm.h"
#include "kmditaskbar.h"
#include "kmdichildfrm.h"
#include "kmdichildarea.h"
#include "kmdichildview.h"
#include "kmditoolviewaccessor_p.h"
#include "kmdifocuslist.h"
#include "kmdidocumentviewtabwidget.h"
#include "kmdiguiclient.h"

#include "win_undockbutton.xpm"
#include "win_minbutton.xpm"
#include "win_restorebutton.xpm"
#include "win_closebutton.xpm"
#include "kde_undockbutton.xpm"
#include "kde_minbutton.xpm"
#include "kde_restorebutton.xpm"
#include "kde_closebutton.xpm"
#include "kde2_undockbutton.xpm"
#include "kde2_minbutton.xpm"
#include "kde2_restorebutton.xpm"
#include "kde2_closebutton.xpm"
#include "kde2laptop_undockbutton.xpm"
#include "kde2laptop_minbutton.xpm"
#include "kde2laptop_restorebutton.xpm"
#include "kde2laptop_closebutton.xpm"
#include "kde2laptop_closebutton_menu.xpm"

#if defined Q_WS_X11 && ! defined K_WS_QTONLY
#ifndef NO_KDE
#include <X11/X.h> // schroder
#include <X11/Xlib.h> // schroder
#endif

#ifdef KeyRelease
/* I hate the defines in the X11 header files. Get rid of one of them */
#undef KeyRelease
#endif

#ifdef KeyPress
/* I hate the defines in the X11 header files. Get rid of one of them */
#undef KeyPress
#endif
#endif // Q_WS_X11 && ! K_WS_QTONLY

using namespace KParts;

KMdi::FrameDecor KMdiMainFrm::m_frameDecoration = KMdi::KDELook;

//KMdi::MdiMode KMdiMainFrm::m_mdiMode = KMdi::ChildframeMode;

class KMdiMainFrmPrivate {
public:
  KMdiMainFrmPrivate(): focusList(0) {
    for (int i=0;i<4;i++) {
      activeDockPriority[i]=0;
      m_styleIDEAlMode = 0;
      m_toolviewStyle = 0;
    }
  }
  ~KMdiMainFrmPrivate() {}
  KMdiDockContainer* activeDockPriority[4];
  KMdiFocusList *focusList;
  int m_styleIDEAlMode;
  int m_toolviewStyle;
};

//============ constructor ============//
KMdiMainFrm::KMdiMainFrm(QWidget* parentWidget, const char* name, KMdi::MdiMode mdiMode,WFlags flags)
: KParts::DockMainWindow( parentWidget, name, flags)
   ,m_mdiMode(KMdi::UndefinedMode)
   ,m_pMdi(0L)
   ,m_pTaskBar(0L)
   ,m_pDocumentViews(0L)
   ,m_pCurrentWindow(0L)
   ,m_pWindowPopup(0L)
   ,m_pTaskBarPopup(0L)
   ,m_pWindowMenu(0L)
   ,m_pDockMenu(0L)
   ,m_pMdiModeMenu(0L)
   ,m_pPlacingMenu(0L)
   ,m_pMainMenuBar(0L)
   ,m_pUndockButtonPixmap(0L)
   ,m_pMinButtonPixmap(0L)
   ,m_pRestoreButtonPixmap(0L)
   ,m_pCloseButtonPixmap(0L)
   ,m_pUndock(0L)
   ,m_pMinimize(0L)
   ,m_pRestore(0L)
   ,m_pClose(0L)
   ,m_bMaximizedChildFrmMode(false)
   ,m_oldMainFrmHeight(0)
   ,m_oldMainFrmMinHeight(0)
   ,m_oldMainFrmMaxHeight(0)
   ,m_bSDIApplication(false)
   ,m_pDockbaseAreaOfDocumentViews(0L)
   ,m_pTempDockSession(0L)
   ,m_bClearingOfWindowMenuBlocked(false)
   ,m_pDragEndTimer(0L)
   ,m_bSwitching(false)
   ,m_leftContainer(0)
   ,m_rightContainer(0)
   ,m_topContainer(0)
   ,m_bottomContainer(0)
   ,d(new KMdiMainFrmPrivate())
   ,m_mdiGUIClient(0)
   ,m_managedDockPositionMode(false)
   ,m_documentTabWidget(0)
{
//  kdDebug()<<"=== KMdiMainFrm() ==="<<endl;
  // Create the local lists of windows
  m_pDocumentViews = new QPtrList<KMdiChildView>;
  m_pDocumentViews->setAutoDelete(false);
  m_pToolViews = new QMap<QWidget*,KMdiToolViewAccessor*>;

  // This seems to be needed (re-check it after Qt2.0 comed out)
  setFocusPolicy(ClickFocus);

  // create the central widget
  createMdiManager();

  // cover KMdi's childarea by a dockwidget
  m_pDockbaseAreaOfDocumentViews = createDockWidget( "mdiAreaCover", QPixmap(), 0L, "mdi_area_cover");
  m_pDockbaseAreaOfDocumentViews->setDockWindowTransient(this,true);
  m_pDockbaseAreaOfDocumentViews->setEnableDocking(KDockWidget::DockNone);
  m_pDockbaseAreaOfDocumentViews->setDockSite(KDockWidget::DockCorner);
  m_pDockbaseAreaOfDocumentViews->setWidget(m_pMdi);
  // set this dock to main view
  setView(m_pDockbaseAreaOfDocumentViews);
  setMainDockWidget(m_pDockbaseAreaOfDocumentViews);

  // Apply options for the MDI manager
  applyOptions();

  m_pTaskBarPopup=new QPopupMenu( this, "taskbar_popup_menu");
  m_pWindowPopup=new QPopupMenu( this, "window_popup_menu");

  m_pWindowMenu = new QPopupMenu( this, "window_menu");
  m_pWindowMenu->setCheckable( true);
  QObject::connect( m_pWindowMenu, SIGNAL(aboutToShow()), this, SLOT(fillWindowMenu()) );

  m_pDockMenu = new QPopupMenu( this, "dock_menu");
  m_pDockMenu->setCheckable( true);

  m_pMdiModeMenu = new QPopupMenu( this, "mdimode_menu");
  m_pMdiModeMenu->setCheckable( true);

  m_pPlacingMenu = new QPopupMenu( this, "placing_menu");

  // the MDI view taskbar
  createTaskBar();

  // this is only a hack, but prevents us from crash because the buttons are otherwise
  // not created before we switch the modes where we need them !!!
  setMenuForSDIModeSysButtons(menuBar());

  switch (mdiMode) {
    case KMdi::IDEAlMode:
//      kdDebug(760)<<"switch(mdiMode): IDEAlMode"<<endl;
      switchToIDEAlMode();
      break;
    case KMdi::TabPageMode:
//      kdDebug(760)<<"switch(mdiMode): TabPageMode"<<endl;
      switchToTabPageMode();
      break;
    case KMdi::ToplevelMode:
//      kdDebug(760)<<"switch(mdiMode): TopLevelMode"<<endl;
      switchToToplevelMode();
      break;
    default:
      m_mdiMode=KMdi::ChildframeMode;
//      kdDebug(760)<<"switch(mdiMode): default"<<endl;
      break;
  }

  // drag end timer
  m_pDragEndTimer = new QTimer();
  connect(m_pDragEndTimer, SIGNAL(timeout()), this, SLOT(dragEndTimeOut()));
}

void KMdiMainFrm::setStandardMDIMenuEnabled(bool showModeMenu) {
  m_mdiGUIClient=new KMDIPrivate::KMDIGUIClient(this,showModeMenu);
  connect(m_mdiGUIClient,SIGNAL(toggleTop()),this,SIGNAL(toggleTop()));
  connect(m_mdiGUIClient,SIGNAL(toggleLeft()),this,SIGNAL(toggleLeft()));
  connect(m_mdiGUIClient,SIGNAL(toggleRight()),this,SIGNAL(toggleRight()));
  connect(m_mdiGUIClient,SIGNAL(toggleBottom()),this,SIGNAL(toggleBottom()));

  if (m_mdiMode==KMdi::IDEAlMode) {
    if (m_topContainer)
      connect(this,SIGNAL(toggleTop()),m_topContainer->getWidget(),SLOT(toggle()));
    if (m_leftContainer)
      connect(this,SIGNAL(toggleLeft()),m_leftContainer->getWidget(),SLOT(toggle()));
    if (m_rightContainer)
      connect(this,SIGNAL(toggleRight()),m_rightContainer->getWidget(),SLOT(toggle()));
    if (m_bottomContainer)
      connect(this,SIGNAL(toggleBottom()),m_bottomContainer->getWidget(),SLOT(toggle()));
  }

  emit mdiModeHasBeenChangedTo(m_mdiMode);
}

//============ ~KMdiMainFrm ============//
KMdiMainFrm::~KMdiMainFrm()
{
  delete d;
  d=0;
  //save the children first to a list, as removing invalidates our iterator
  QValueList<KMdiChildView *> children;
  for(KMdiChildView *w = m_pDocumentViews->first();w;w= m_pDocumentViews->next()){
    children.append(w);
  }
  // safely close the windows so properties are saved...
  QValueListIterator<KMdiChildView *> childIt;
  for (childIt = children.begin(); childIt != children.end(); ++childIt)
  {
    closeWindow(*childIt, false); // without re-layout taskbar!
  }

  emit lastChildViewClosed();
  delete m_pDocumentViews;
  delete m_pToolViews;
  m_pToolViews=0;
  delete m_pDragEndTimer;

  delete m_pUndockButtonPixmap;
  delete m_pMinButtonPixmap;
  delete m_pRestoreButtonPixmap;
  delete m_pCloseButtonPixmap;

  //deletes added for Release-Version-Pop-Up-WinMenu-And-Go-Out-Problem
  delete m_pDockMenu;
  delete m_pMdiModeMenu;
  delete m_pPlacingMenu;
  delete m_pTaskBarPopup;
  delete m_pWindowPopup;
  delete m_pWindowMenu;
  delete m_mdiGUIClient;
  m_mdiGUIClient=0;
}

//============ applyOptions ============//
#ifdef __GNUC__
#warning fixme
#endif
void KMdiMainFrm::applyOptions()
{
  for(KMdiChildView *w = m_pDocumentViews->first();w;w= m_pDocumentViews->next()){
    QWidget *wdgt = w;
    if(w->mdiParent())wdgt =w->mdiParent();
    // Really ugly hack to FORCE the resize event
    // a resize(width(),height()) won't work...
    wdgt->resize(wdgt->width()+1,wdgt->height()+1);
    wdgt->resize(wdgt->width()-1,wdgt->height()-1);
  }
}

//============ createMdiManager ============//
void KMdiMainFrm::createMdiManager()
{
  m_pMdi=new KMdiChildArea(this);
  setCentralWidget(m_pMdi);
  QObject::connect( m_pMdi, SIGNAL(nowMaximized(bool)), this, SLOT(setEnableMaximizedChildFrmMode(bool)) );
  QObject::connect( m_pMdi, SIGNAL(noMaximizedChildFrmLeft(KMdiChildFrm*)), this, SLOT(switchOffMaximizeModeForMenu(KMdiChildFrm*)) );
  QObject::connect( m_pMdi, SIGNAL(sysButtonConnectionsMustChange(KMdiChildFrm*,KMdiChildFrm*)), this, SLOT(updateSysButtonConnections(KMdiChildFrm*,KMdiChildFrm*)) );
  QObject::connect( m_pMdi, SIGNAL(popupWindowMenu(QPoint)), this, SLOT(popupWindowMenu(QPoint)) );
  QObject::connect( m_pMdi, SIGNAL(lastChildFrmClosed()), this, SIGNAL(lastChildFrmClosed()) );
}

//============ createTaskBar ==============//
void KMdiMainFrm::createTaskBar()
{
  m_pTaskBar = new KMdiTaskBar(this,QMainWindow::DockBottom);
  m_pTaskBar->installEventFilter( this);
}

void KMdiMainFrm::slot_toggleTaskBar()
{
  if (!m_pTaskBar)
    return;
  m_pTaskBar->switchOn( !m_pTaskBar->isSwitchedOn());
}

void KMdiMainFrm::resizeEvent(QResizeEvent *e)
{
  if( (m_mdiMode == KMdi::ToplevelMode) && !parentWidget())
    if( e->oldSize().height() != e->size().height()) {
      return;
    }
  KParts::DockMainWindow::resizeEvent(e);
  if (!m_mdiGUIClient) return;
  setSysButtonsAtMenuPosition();
}

//================ setMinimumSize ===============//

void KMdiMainFrm::setMinimumSize( int minw, int minh)
{
  if( (m_mdiMode == KMdi::ToplevelMode) && !parentWidget())
    return;
  DockMainWindow::setMinimumSize( minw, minh);
}

//================ wrapper ===============//

KMdiChildView* KMdiMainFrm::createWrapper(QWidget *view, const QString& name, const QString& shortName)
{
  Q_ASSERT( view ); // if this assert fails, then some part didn't return a widget. Fix the part ;)

  KMdiChildView* pMDICover = new KMdiChildView(name, // caption
      0L, // parent
      name.latin1()); // object name, necessary later in the dockwidgets
  QBoxLayout* pLayout = new QHBoxLayout( pMDICover, 0, -1, "layout");
  view->reparent(pMDICover, QPoint(0,0));
  pLayout->addWidget(view);
//  pMDICover->setName(name);
  pMDICover->setTabCaption(shortName);
  pMDICover->setCaption(name);

  const QPixmap* wndIcon = view->icon();
  if (wndIcon) {
    pMDICover->setIcon(*wndIcon);
  }
  pMDICover->trackIconAndCaptionChanges(view);
  return pMDICover;
}

//================ addWindow ===============//

void KMdiMainFrm::addWindow( KMdiChildView* pWnd, int flags)
{
  addWindow(pWnd, flags, -1);
}

void KMdiMainFrm::addWindow( KMdiChildView* pWnd, int flags, int index)
{
  if( windowExists( pWnd,AnyView)) {
    // is already added to the MDI system
    return;
  }

  if( flags & KMdi::ToolWindow) {
    addToolWindow( pWnd);
    // some kind of cascading
    pWnd->move(m_pMdi->mapToGlobal(m_pMdi->getCascadePoint()));

    return;
  }

  // common connections used when under MDI control
  QObject::connect( pWnd, SIGNAL(clickedInWindowMenu(int)), this, SLOT(windowMenuItemActivated(int)) );
  QObject::connect( pWnd, SIGNAL(focusInEventOccurs(KMdiChildView*)), this, SLOT(activateView(KMdiChildView*)) );
  QObject::connect( pWnd, SIGNAL(childWindowCloseRequest(KMdiChildView*)), this, SLOT(childWindowCloseRequest(KMdiChildView*)) );
  QObject::connect( pWnd, SIGNAL(attachWindow(KMdiChildView*,bool)), this, SLOT(attachWindow(KMdiChildView*,bool)) );
  QObject::connect( pWnd, SIGNAL(detachWindow(KMdiChildView*,bool)), this, SLOT(detachWindow(KMdiChildView*,bool)) );
  QObject::connect( pWnd, SIGNAL(clickedInDockMenu(int)), this, SLOT(dockMenuItemActivated(int)) );
  connect(pWnd,SIGNAL(activated(KMdiChildView*)),this,SIGNAL(viewActivated(KMdiChildView*)));
  connect(pWnd,SIGNAL(deactivated(KMdiChildView*)),this,SIGNAL(viewDeactivated(KMdiChildView*)));
  if (index == -1)
    m_pDocumentViews->append(pWnd);
  else
    m_pDocumentViews->insert(index, pWnd);
  if (m_pTaskBar) {
    KMdiTaskBarButton* but = m_pTaskBar->addWinButton(pWnd);
    QObject::connect( pWnd, SIGNAL(tabCaptionChanged(const QString&)), but, SLOT(setNewText(const QString&)) );
  }

  // embed the view depending on the current MDI mode
  if (m_mdiMode == KMdi::TabPageMode || m_mdiMode == KMdi::IDEAlMode) {
    //      const QPixmap& wndIcon = pWnd->icon() ? *(pWnd->icon()) : QPixmap();

    m_documentTabWidget->insertTab(pWnd, pWnd->icon() ? *(pWnd->icon()) : QPixmap(),pWnd->tabCaption(), index);

    /*
       connect(pWnd,SIGNAL(iconOrCaptionUdpated(QWidget*,QPixmap,const QString&)),
       m_documentTabWidget,SLOT(updateView(QWidget*,QPixmap,const QString&)));
     */
    connect( pWnd, SIGNAL(iconUpdated(QWidget*, QPixmap )), m_documentTabWidget, SLOT(updateIconInView(QWidget*, QPixmap )) );
    connect( pWnd, SIGNAL(captionUpdated(QWidget*, const QString& )), m_documentTabWidget, SLOT(updateCaptionInView(QWidget*, const QString& )) );
#if 0
    KDockWidget* pCover = createDockWidget( pWnd->name(),
        wndIcon,
        0L,  // parent
        pWnd->caption(),
        pWnd->tabCaption());
    pCover->setWidget( pWnd);
    pCover->setToolTipString( pWnd->caption());
    pCover->setDockWindowTransient(this,true);
    if (!(flags & KMdi::Detach)) {
      m_pDockbaseOfTabPage->setDockSite(KDockWidget::DockFullSite);
      pCover->manualDock( m_pDockbaseOfTabPage, KDockWidget::DockCenter);
      pCover->setEnableDocking(KDockWidget::DockNone);
      if (m_pDockbaseOfTabPage == m_pDockbaseAreaOfDocumentViews) {
        m_pDockbaseAreaOfDocumentViews->undock();
        m_pDockbaseAreaOfDocumentViews->setDockSite(KDockWidget::DockNone);
      }
      else {
        m_pDockbaseOfTabPage->setDockSite(KDockWidget::DockCorner);
      }
      m_pDockbaseOfTabPage = (KDockWidget*) pCover;
    }
    if (!(flags & KMdi::Hide)) {
      pCover->show();
    }
    pWnd->setFocus();
    if (m_pDocumentViews->count() == 1) {
      m_pClose->show();  // show the close button in case it isn't already
    }
#endif
  } else {
    if( (flags & KMdi::Detach) || (m_mdiMode == KMdi::ToplevelMode)) {
      detachWindow( pWnd, !(flags & KMdi::Hide));
      emit childViewIsDetachedNow(pWnd); // fake it because detach won't call it in this case of addWindow-to-MDI
    } else {
      attachWindow( pWnd, !(flags & KMdi::Hide), flags & KMdi::UseKMdiSizeHint);
    }

    if ((m_bMaximizedChildFrmMode && !(m_bSDIApplication && (flags & KMdi::Detach)) && (m_mdiMode != KMdi::ToplevelMode))
        || (flags & KMdi::Maximize) ) {
      if (!pWnd->isMaximized())
        pWnd->maximize();
    }
    if (!m_bSDIApplication || (flags & KMdi::Detach)) {
      if (flags & KMdi::Minimize)
      {
        pWnd->minimize();
      }
      if (!(flags & KMdi::Hide)) {
        if (pWnd->isAttached()) {
          pWnd->mdiParent()->show();
        } else {
          pWnd->show();
        }
      }
    }
  }
}

//============ addWindow ============//
void KMdiMainFrm::addWindow( KMdiChildView* pWnd, QRect rectNormal, int flags)
{
  addWindow( pWnd, flags);
  if (m_bMaximizedChildFrmMode && pWnd->isAttached()) {
    pWnd->setRestoreGeometry( rectNormal);
  } else {
    pWnd->setGeometry( rectNormal);
  }
}

//============ addWindow ============//
void KMdiMainFrm::addWindow( KMdiChildView* pWnd, QPoint pos, int flags)
{
  addWindow( pWnd, flags);
  if (m_bMaximizedChildFrmMode && pWnd->isAttached()) {
    pWnd->setRestoreGeometry( QRect(pos, pWnd->restoreGeometry().size()));
  } else {
    pWnd->move( pos);
  }
}



KMdiToolViewAccessor *KMdiMainFrm::createToolWindow()
{
  return new KMdiToolViewAccessor(this);
}


void KMdiMainFrm::deleteToolWindow( QWidget* pWnd) {
  if (m_pToolViews->contains(pWnd)) {
    deleteToolWindow((*m_pToolViews)[pWnd]);
  }
}

void KMdiMainFrm::deleteToolWindow( KMdiToolViewAccessor *accessor) {
  if (!accessor) return;
  delete accessor;
}

//============ addWindow ============//
KMdiToolViewAccessor *KMdiMainFrm::addToolWindow( QWidget* pWnd, KDockWidget::DockPosition pos, QWidget* pTargetWnd, int percent, const QString& tabToolTip, const QString& tabCaption)
{
  QWidget *tvta=pWnd;
  KDockWidget* pDW = dockManager->getDockWidgetFromName(pWnd->name());
  if (pDW) {
    // probably readDockConfig already created the widgetContainer, use that
    pDW->setWidget(pWnd);

    if (pWnd->icon()) {
      pDW->setPixmap(*pWnd->icon());
    }
    pDW->setTabPageLabel((tabCaption==0)?pWnd->caption():tabCaption);
    pDW->setToolTipString(tabToolTip);
    dockManager->removeFromAutoCreateList(pDW);
    pWnd=pDW;
  }

  QRect r=pWnd->geometry();

  KMdiToolViewAccessor *mtva=new KMdiToolViewAccessor(this,pWnd,tabToolTip,(tabCaption==0)?pWnd->caption():tabCaption);
  m_pToolViews->insert(tvta,mtva);

  if (pos == KDockWidget::DockNone) {
    mtva->d->widgetContainer->setEnableDocking(KDockWidget::DockNone);
    mtva->d->widgetContainer->reparent(this, Qt::WType_TopLevel | Qt::WType_Dialog, r.topLeft(), true); //pToolView->isVisible());
  } else {   // add (and dock) the toolview as DockWidget view
    //const QPixmap& wndIcon = pWnd->icon() ? *(pWnd->icon()) : QPixmap();

    //KDockWidget *pCover=mtva->d->widgetContainer;

    mtva->place(pos, pTargetWnd,percent);

  }

  return mtva;
}

//============ attachWindow ============//
void KMdiMainFrm::attachWindow(KMdiChildView *pWnd, bool bShow, bool bAutomaticResize)
{
  pWnd->installEventFilter(this);

  // decide whether window shall be cascaded
  bool bCascade = false;
  QApplication::sendPostedEvents();
  QRect frameGeo = pWnd->frameGeometry();
  QPoint topLeftScreen = pWnd->mapToGlobal(QPoint(0,0));
  QPoint topLeftMdiChildArea = m_pMdi->mapFromGlobal(topLeftScreen);
  QRect childAreaGeo = m_pMdi->geometry();
  if ( (topLeftMdiChildArea.x() < 0) || (topLeftMdiChildArea.y() < 0) ||
      (topLeftMdiChildArea.x()+frameGeo.width() > childAreaGeo.width()) ||
      (topLeftMdiChildArea.y()+frameGeo.height() > childAreaGeo.height()) ) {
    bCascade = true;
  }

  // create frame and insert child view
  KMdiChildFrm *lpC=new KMdiChildFrm(m_pMdi);
  pWnd->hide();
  if (!bCascade) {
    lpC->move(topLeftMdiChildArea);
  }
  lpC->setClient(pWnd, bAutomaticResize);
  lpC->setFocus();
  pWnd->youAreAttached(lpC);
  if( (m_mdiMode == KMdi::ToplevelMode) && !parentWidget()) {
    setMinimumHeight( m_oldMainFrmMinHeight);
    setMaximumHeight( m_oldMainFrmMaxHeight);
    resize( width(), m_oldMainFrmHeight);
    m_oldMainFrmHeight = 0;
    switchToChildframeMode();
  }

  m_pMdi->manageChild(lpC,false,bCascade);
  if (m_pMdi->topChild() && m_pMdi->topChild()->isMaximized()) {
    QRect r = lpC->geometry();
    lpC->setGeometry(-lpC->m_pClient->x(), -lpC->m_pClient->y(),
        m_pMdi->width()  + KMDI_CHILDFRM_DOUBLE_BORDER,
        m_pMdi->height() + lpC->captionHeight() + KMDI_CHILDFRM_SEPARATOR + KMDI_CHILDFRM_DOUBLE_BORDER);
    lpC->setRestoreGeometry(r);
  }

  if (bShow) {
    lpC->show();
  }

#undef FocusIn
  QFocusEvent fe(QEvent::FocusIn);
  QApplication::sendEvent( pWnd, &fe);

  m_pCurrentWindow  = pWnd;  // required for checking the active item
}

//============= detachWindow ==============//
void KMdiMainFrm::detachWindow(KMdiChildView *pWnd, bool bShow)
{
  if (pWnd->isAttached()) {
    pWnd->removeEventFilter(this);
    pWnd->youAreDetached();
    // this is only if it was attached and you want to detach it
    if (pWnd->parent()) {
      KMdiChildFrm *lpC=pWnd->mdiParent();
      if (lpC) {
        if (lpC->icon()) {
          QPixmap pixm(*(lpC->icon()));
          pWnd->setIcon(pixm);
        }
        QString capt(lpC->caption());
        if (!bShow)
          lpC->hide();
        lpC->unsetClient( m_undockPositioningOffset);
        m_pMdi->destroyChildButNotItsView(lpC,false); //Do not focus the new top child , we loose focus...
        pWnd->setCaption(capt);
      }
    }
  }
  else {
    if (pWnd->size().isEmpty() || (pWnd->size() == QSize(1,1))) {
      if (m_pCurrentWindow) {
        pWnd->setGeometry( QRect( m_pMdi->getCascadePoint(m_pDocumentViews->count()-1), m_pCurrentWindow->size()));
      }
      else {
        pWnd->setGeometry( QRect( m_pMdi->getCascadePoint(m_pDocumentViews->count()-1), defaultChildFrmSize()));
      }
    }
#if defined Q_WS_X11 && ! defined K_WS_QTONLY
    if (mdiMode() == KMdi::ToplevelMode) {
      XSetTransientForHint(qt_xdisplay(),pWnd->winId(),topLevelWidget()->winId());
    }
#endif

    return;
  }

#if defined Q_WS_X11 && ! defined K_WS_QTONLY
  if (mdiMode() == KMdi::ToplevelMode) {
    XSetTransientForHint(qt_xdisplay(),pWnd->winId(),topLevelWidget()->winId());
  }
#endif

  // this will show it...
  if (bShow) {
    activateView(pWnd);
  }

  emit childViewIsDetachedNow(pWnd);
}

//============== removeWindowFromMdi ==============//
void KMdiMainFrm::removeWindowFromMdi(KMdiChildView *pWnd)
{
Q_UNUSED(pWnd)
   //Closes a child window. sends no close event : simply deletes it
#ifdef __GNUC__
#warning FIXME FIXME FIXME
#endif
#if 0
   if (!(m_pWinList->removeRef(pWnd)))
      return;
   if (m_pWinList->count() == 0)
     m_pCurrentWindow = 0L;

   QObject::disconnect( pWnd, SIGNAL(attachWindow(KMdiChildView*,bool)), this, SLOT(attachWindow(KMdiChildView*,bool)) );
   QObject::disconnect( pWnd, SIGNAL(detachWindow(KMdiChildView*,bool)), this, SLOT(detachWindow(KMdiChildView*,bool)) );
   QObject::disconnect( pWnd, SIGNAL(focusInEventOccurs(KMdiChildView*)), this, SLOT(activateView(KMdiChildView*)) );
   QObject::disconnect( pWnd, SIGNAL(childWindowCloseRequest(KMdiChildView*)), this, SLOT(childWindowCloseRequest(KMdiChildView*)) );
   QObject::disconnect( pWnd, SIGNAL(clickedInWindowMenu(int)), this, SLOT(windowMenuItemActivated(int)) );
   QObject::disconnect( pWnd, SIGNAL(clickedInDockMenu(int)), this, SLOT(dockMenuItemActivated(int)) );

   if (m_pTaskBar) {
      KMdiTaskBarButton* but = m_pTaskBar->getButton(pWnd);
      if (but != 0L) {
         QObject::disconnect( pWnd, SIGNAL(tabCaptionChanged(const QString&)), but, SLOT(setNewText(const QString&)) );
      }
      m_pTaskBar->removeWinButton(pWnd);
   }

   if (m_mdiMode == KMdi::TabPageMode) {
      if (m_pWinList->count() == 0) {
         if (!m_pDockbaseAreaOfDocumentViews) {
            m_pDockbaseAreaOfDocumentViews = createDockWidget( "mdiAreaCover", QPixmap(), 0L, "mdi_area_cover");
            m_pDockbaseAreaOfDocumentViews->setDockWindowTransient(this,true);

            m_pDockbaseAreaOfDocumentViews->setWidget(m_pMdi);
            setMainDockWidget(m_pDockbaseAreaOfDocumentViews);
         }
         m_pDockbaseOfTabPage->setDockSite(KDockWidget::DockFullSite);
         m_pDockbaseAreaOfDocumentViews->setEnableDocking(KDockWidget::DockCenter);
         m_pDockbaseAreaOfDocumentViews->manualDock( m_pDockbaseOfTabPage, KDockWidget::DockCenter);
         m_pDockbaseAreaOfDocumentViews->setEnableDocking(KDockWidget::DockNone);
         m_pDockbaseOfTabPage = m_pDockbaseAreaOfDocumentViews;
         m_pClose->hide();
      }
      KDockWidget* pDockW = (KDockWidget*) pWnd->parentWidget();
      pWnd->reparent(0L, QPoint(0,0));
      pDockW->setWidget(0L);
      if (pDockW == m_pDockbaseOfTabPage) {
         QTabWidget* pTab = (QTabWidget*) pDockW->parentWidget()->parentWidget();
         int cnt = pTab->count();
         m_pDockbaseOfTabPage = (KDockWidget*) pTab->page(cnt - 2);
         if (pDockW == m_pDockbaseOfTabPage) {
            m_pDockbaseOfTabPage = (KDockWidget*) pTab->page(cnt - 1); // different to the one deleted next
         }
      }
      delete pDockW;
      if (m_pWinList->count() == 1) {
         m_pWinList->last()->activate(); // all other views are activated by tab switch
      }
   }
   else if (pWnd->isAttached()) {
      pWnd->mdiParent()->hide();
      m_pMdi->destroyChildButNotItsView(pWnd->mdiParent());
   }
   else {
      // is not attached
      if (m_pMdi->getVisibleChildCount() > 0) {
         setActiveWindow();
         m_pCurrentWindow = 0L;
         KMdiChildView* pView = m_pMdi->topChild()->m_pClient;
         if (pView) {
            pView->activate();
         }
      }
      else if (m_pWinList->count() > 0) {
//crash?         m_pWinList->last()->activate();
//crash?         m_pWinList->last()->setFocus();
      }
   }

   if (pWnd->isToolView())
      pWnd->m_bToolView = false;

   if (!m_pCurrentWindow)
      emit lastChildViewClosed();
#endif
}

//============== closeWindow ==============//
void KMdiMainFrm::closeWindow(KMdiChildView *pWnd, bool layoutTaskBar)
{
  if (!pWnd) return;
  //Closes a child window. sends no close event : simply deletes it
  m_pDocumentViews->removeRef(pWnd);
  if (m_pDocumentViews->count() == 0)
    m_pCurrentWindow = 0L;

  if (m_pTaskBar) {
    m_pTaskBar->removeWinButton(pWnd, layoutTaskBar);
  }

  if ((m_mdiMode == KMdi::TabPageMode) || (m_mdiMode==KMdi::IDEAlMode)) {
    if (!m_documentTabWidget) return; //oops
    if (m_pDocumentViews->count()==0) m_pClose->hide();
    pWnd->reparent(0L, QPoint(0,0));
//    kdDebug() << "-------- 1" << endl;
    if (m_pDocumentViews->count() == 1) {
      m_pDocumentViews->last()->activate(); // all other views are activated by tab switch
    }
  }
  if ((m_mdiMode == KMdi::TabPageMode) || (m_mdiMode==KMdi::IDEAlMode)) {
    if (m_pDocumentViews->count() == 0) {
      if (!m_pDockbaseAreaOfDocumentViews) {
        m_pDockbaseAreaOfDocumentViews = createDockWidget( "mdiAreaCover", QPixmap(), 0L, "mdi_area_cover");
        m_pDockbaseAreaOfDocumentViews->setDockWindowTransient(this,true);
        m_pDockbaseAreaOfDocumentViews->setWidget(m_pMdi);
        setMainDockWidget(m_pDockbaseAreaOfDocumentViews);
      }
#if 0
      m_pDockbaseOfTabPage->setDockSite(KDockWidget::DockFullSite);
      m_pDockbaseAreaOfDocumentViews->setEnableDocking(KDockWidget::DockCenter);
      m_pDockbaseAreaOfDocumentViews->manualDock( m_pDockbaseOfTabPage, KDockWidget::DockCenter);
      m_pDockbaseAreaOfDocumentViews->setEnableDocking(KDockWidget::DockNone);
      m_pDockbaseOfTabPage = m_pDockbaseAreaOfDocumentViews;
#endif
      m_pClose->hide();
    }
#if 0
    KDockWidget* pDockW = (KDockWidget*) pWnd->parentWidget();
    pWnd->reparent(0L, QPoint(0,0));
    pDockW->setWidget(0L);
    if (pDockW == m_pDockbaseOfTabPage) {
      QTabWidget* pTab = (QTabWidget*) pDockW->parentWidget()->parentWidget();
      int cnt = pTab->count();
      m_pDockbaseOfTabPage = (KDockWidget*) pTab->page(cnt - 2);
      if (pDockW == m_pDockbaseOfTabPage) {
        m_pDockbaseOfTabPage = (KDockWidget*) pTab->page(cnt - 1); // different to the one deleted next
      }
    }
    delete pDockW;
#endif
    delete pWnd;
    if (m_pDocumentViews->count() == 1) {
      m_pDocumentViews->last()->activate(); // all other views are activated by tab switch
    }
  } else if (pWnd->isAttached()) {
    m_pMdi->destroyChild(pWnd->mdiParent());
  } else {
    delete pWnd;
    // is not attached
    if (m_pMdi->getVisibleChildCount() > 0) {
      setActiveWindow();
      m_pCurrentWindow = 0L;
      KMdiChildView* pView = m_pMdi->topChild()->m_pClient;
      if (pView) {
        pView->activate();
      }
    } else if (m_pDocumentViews->count() > 0) {
      if (m_pDocumentViews->current()) {
        m_pDocumentViews->current()->activate();
        m_pDocumentViews->current()->setFocus();
      }
      else {
        m_pDocumentViews->last()->activate();
        m_pDocumentViews->last()->setFocus();
      }
    }
  }

  if (!m_pCurrentWindow)
    emit lastChildViewClosed();
}

//================== findWindow =================//
KMdiChildView * KMdiMainFrm::findWindow(const QString& caption)
{
  for(KMdiChildView *w=m_pDocumentViews->first();w;w=m_pDocumentViews->next()){
    if(QString(w->caption()) == QString(caption))return w;   //F.B.
  }
  return 0L;
}

//================== activeWindow ===================//
KMdiChildView * KMdiMainFrm::activeWindow()
{
  return m_pCurrentWindow;
}

//================== windowExists ? =================//
bool KMdiMainFrm::windowExists(KMdiChildView *pWnd, ExistsAs as)
{
  if ((as==ToolView) || (as==AnyView)) {
    if (m_pToolViews->contains(pWnd)) return true;
    if (as==ToolView) return false;
  }

  for(KMdiChildView *w=m_pDocumentViews->first();w;w=m_pDocumentViews->next()){
    if (w == pWnd) return true;
  }

  return false;
}

QPopupMenu * KMdiMainFrm::windowPopup(KMdiChildView * pWnd,bool bIncludeTaskbarPopup)
{
  m_pWindowPopup->clear();
  if(bIncludeTaskbarPopup){
    m_pWindowPopup->insertItem(i18n("Window"),taskBarPopup(pWnd,false));
    m_pWindowPopup->insertSeparator();
  }
  return m_pWindowPopup;
}

//================ taskBarPopup =================//
QPopupMenu * KMdiMainFrm::taskBarPopup(KMdiChildView *pWnd,bool /*bIncludeWindowPopup*/)
{
  //returns the g_pTaskBarPopup filled according to the KMdiChildView state
  m_pTaskBarPopup->clear();
  if(pWnd->isAttached()){
    m_pTaskBarPopup->insertItem(i18n("Undock"),pWnd,SLOT(detach()));
    m_pTaskBarPopup->insertSeparator();
    if(pWnd->isMinimized() || pWnd->isMaximized())
      m_pTaskBarPopup->insertItem(i18n("Restore"),pWnd,SLOT(restore()));
    if(!pWnd->isMaximized())m_pTaskBarPopup->insertItem(i18n("Maximize"),pWnd,SLOT(maximize()));
    if(!pWnd->isMinimized())m_pTaskBarPopup->insertItem(i18n("Minimize"),pWnd,SLOT(minimize()));
  } else m_pTaskBarPopup->insertItem(i18n("Dock"),pWnd,SLOT(attach()));
  m_pTaskBarPopup->insertSeparator();
  m_pTaskBarPopup->insertItem(i18n("Close"),pWnd,SLOT(close()));
  // the window has a view...get the window popup
  m_pTaskBarPopup->insertSeparator();
  m_pTaskBarPopup->insertItem(i18n("Operations"),windowPopup(pWnd,false));  //alvoid recursion
  return m_pTaskBarPopup;
}

void KMdiMainFrm::slotDocCurrentChanged(QWidget* pWidget)
{
  KMdiChildView *pWnd = static_cast<KMdiChildView*>(pWidget);
  pWnd->m_bMainframesActivateViewIsPending = true;

  bool bActivateNecessary = true;
  if (m_pCurrentWindow != pWnd) {
    m_pCurrentWindow = pWnd;
  }

  if (m_pTaskBar) {
    m_pTaskBar->setActiveButton(pWnd);
  }

  if (m_documentTabWidget && m_mdiMode == KMdi::TabPageMode || m_mdiMode==KMdi::IDEAlMode) {
    m_documentTabWidget->showPage(pWnd);
    pWnd->activate();
  }
  else {
    if (pWnd->isAttached()) {
        if (bActivateNecessary && (m_pMdi->topChild() == pWnd->mdiParent())) {
          pWnd->activate();
        }
        pWnd->mdiParent()->raiseAndActivate();
    }
    if (!pWnd->isAttached()) {
        if (bActivateNecessary)
          pWnd->activate();
        m_pMdi->setTopChild(0L); // lose focus in the mainframe window
        if (!pWnd->isActiveWindow()) {
          pWnd->setActiveWindow();
        }
        pWnd->raise();
    }
  }
  emit collapseOverlapContainers();
  pWnd->m_bMainframesActivateViewIsPending = false;
}


void KMdiMainFrm::activateView(KMdiChildView* pWnd)
{
  pWnd->m_bMainframesActivateViewIsPending = true;

  bool bActivateNecessary = true;
  if (m_pCurrentWindow != pWnd) {
    m_pCurrentWindow = pWnd;
  } else {
    bActivateNecessary = false;
    // if this method is called as answer to view->activate(),
    // interrupt it because it's not necessary
    pWnd->m_bInterruptActivation = true;
  }

  if (m_pTaskBar) {
    m_pTaskBar->setActiveButton(pWnd);
  }

  if (m_documentTabWidget && m_mdiMode == KMdi::TabPageMode || m_mdiMode==KMdi::IDEAlMode) {
    m_documentTabWidget->showPage(pWnd);
    pWnd->activate();
  }
#if 0
  if (m_mdiMode == KMdi::TabPageMode) {
    makeWidgetDockVisible(pWnd);
    m_pDockbaseOfTabPage = (KDockWidget*) pWnd->parentWidget();
  }
#endif
  else {
    if (pWnd->isAttached()) {
      if (bActivateNecessary && (m_pMdi->topChild() == pWnd->mdiParent())) {
        pWnd->activate();
      }
      pWnd->mdiParent()->raiseAndActivate();
    }
    if (!pWnd->isAttached()) {
      if (bActivateNecessary)
        pWnd->activate();
      m_pMdi->setTopChild(0L); // lose focus in the mainframe window
      if (!pWnd->isActiveWindow()) {
        pWnd->setActiveWindow();
      }
      pWnd->raise();
      //         if (!pWnd->hasFocus()) {
      //            pWnd->setFocus();
      //         }
    }
  }

  emit collapseOverlapContainers();

  pWnd->m_bMainframesActivateViewIsPending = false;
}

void KMdiMainFrm::taskbarButtonRightClicked(KMdiChildView *pWnd)
{
  activateView( pWnd); // set focus
  QApplication::sendPostedEvents();
  taskBarPopup( pWnd, true)->popup( QCursor::pos());
}

void KMdiMainFrm::childWindowCloseRequest(KMdiChildView *pWnd)
{
  KMdiViewCloseEvent* ce = new KMdiViewCloseEvent( pWnd);
  QApplication::postEvent( this, ce);
}

bool KMdiMainFrm::event( QEvent* e)
{
  if( e->type() == QEvent::User) {
    KMdiChildView* pWnd = (KMdiChildView*)((KMdiViewCloseEvent*)e)->data();
    if( pWnd != 0L)
      closeWindow( pWnd);
    return true;
  // A little hack: If MDI child views are moved implicietly by moving
  // the main widget the should know this too. Unfortunately there seems to
  // be no way to catch the move start / move stop situations for the main
  // widget in a clean way. (There is no MouseButtonPress/Release or
  // something like that.) Therefore we do the following: When we get the
  // "first" move event we start a timer and interprete it as "drag begin".
  // If we get the next move event and the timer is running we restart the
  // timer and don't do anything else. If the timer elapses (this meens we
  // haven't had any move event for a while) we interprete this as "drag
  // end". If the moving didn't stop actually, we will later get another
  // "drag begin", so we get a drag end too much, but this would be the same
  // as if the user would stop moving for a little while.
  // Actually we seem to be lucky that the timer does not elapse while we
  // are moving -> so we have no obsolete drag end / begin
  } else if( isVisible() && (e->type() == QEvent::Move)) {
    if (m_pDragEndTimer->isActive()) {
      // this is not the first move -> stop old timer
      m_pDragEndTimer->stop();
    } else {
      // this is the first move -> send the drag begin to all concerned views
      KMdiChildView* pView;
      for (m_pDocumentViews->first(); (pView = m_pDocumentViews->current()) != 0L; m_pDocumentViews->next()) {
        KMdiChildFrmDragBeginEvent    dragBeginEvent(0L);
        QApplication::sendEvent(pView, &dragBeginEvent);
      }
    }
    m_pDragEndTimer->start(200, true); // single shot after 200 ms
  }

  return DockMainWindow::event( e);
}

bool KMdiMainFrm::eventFilter(QObject * /*obj*/, QEvent *e )
{
  if( e->type() == QEvent::FocusIn) {
    QFocusEvent* pFE = (QFocusEvent*) e;
    if (pFE->reason() == QFocusEvent::ActiveWindow) {
      if (m_pCurrentWindow && !m_pCurrentWindow->isHidden() && !m_pCurrentWindow->isAttached() && m_pMdi->topChild()) {
        return true;   // eat the event
      }
    }
    if (m_pMdi) {
      static bool bFocusTCIsPending = false;
      if (!bFocusTCIsPending) {
        bFocusTCIsPending = true;
        m_pMdi->focusTopChild();
        bFocusTCIsPending = false;
      }
    }
  } else if (e->type() == QEvent::KeyRelease) {
    if (switching()) {
      KAction *a = actionCollection()->action( "view_last_window" ) ;
      if (a) {
        const KShortcut cut( a->shortcut() );
        const KKeySequence& seq = cut.seq( 0 );
        const KKey& key = seq.key(0);
        int modFlags = key.modFlags();
        int state = ((QKeyEvent *)e)->state();
        KKey key2( (QKeyEvent *)e );

        /** these are quite some assumptions:
         *   The key combination uses exactly one modifier key
         *   The WIN button in KDE is the meta button in Qt
         **/
        if (state != ((QKeyEvent *)e)->stateAfter() &&
            ((modFlags & KKey::CTRL) > 0) == ((state & Qt::ControlButton) > 0 ) &&
            ((modFlags & KKey::ALT) > 0)  == ((state & Qt::AltButton) > 0)      &&
            ((modFlags & KKey::WIN) > 0)  == ((state & Qt::MetaButton) > 0) )
        {
          activeWindow()->updateTimeStamp();
          setSwitching(false);
        }
        return true;
      } else {
//        kdDebug(9000) << "KAction( \"view_last_window\") not found." << endl;
      }
    }
  }
  return false;  // standard event processing
}

/**
 * close all views
 */
void KMdiMainFrm::closeAllViews()
{
  //save the children first to a list, as removing invalidates our iterator
  QValueList<KMdiChildView *> children;
  for(KMdiChildView *w = m_pDocumentViews->first();w;w= m_pDocumentViews->next()){
    children.append(w);
  }
  QValueListIterator<KMdiChildView *> childIt;
  for (childIt = children.begin(); childIt != children.end(); ++childIt)
  {
    (*childIt)->close();
  }
}


/**
 * iconify all views
 */
void KMdiMainFrm::iconifyAllViews()
{
  for(KMdiChildView *w = m_pDocumentViews->first();w;w= m_pDocumentViews->next())
    w->minimize();
}

/**
 * closes the view of the active (topchild) window
 */
void KMdiMainFrm::closeActiveView()
{
  if( m_pCurrentWindow != 0L) {
    m_pCurrentWindow->close();
  }
}

/** find the root dockwidgets and store their geometry */
void KMdiMainFrm::findRootDockWidgets(QPtrList<KDockWidget>* pRootDockWidgetList, QValueList<QRect>* pPositionList)
{
  if (!pRootDockWidgetList) return;
  if (!pPositionList) return;

  // since we set some windows to toplevel, we must consider the window manager's window frame
  const int frameBorderWidth  = 7;  // @todo: Can we / do we need to ask the window manager?
  const int windowTitleHeight = 10; // @todo:    -"-

  QObjectList* pObjList = queryList( "KDockWidget");
  if (pObjList->isEmpty()) {
    pObjList = queryList( "KDockWidget_Compat::KDockWidget");
  }
  QObjectListIt it( *pObjList);
  QObject* pObj;
  // for all dockwidgets (which are children of this mainwindow)
  while ((pObj = it.current()) != 0L) {
    ++it;
    KDockWidget* pDockW = (KDockWidget*) pObj;
    KDockWidget* pRootDockW = 0L;
    KDockWidget* pUndockCandidate = 0L;
    QWidget* pW = pDockW;
    // find the oldest ancestor of the current dockwidget that can be undocked
    while (!pW->isTopLevel()) {
      if (pW->inherits("KDockWidget") || pW->inherits("KDockWidget_Compat::KDockWidget")) {
        pUndockCandidate = (KDockWidget*) pW;
        if (pUndockCandidate->enableDocking() != KDockWidget::DockNone)
          pRootDockW = pUndockCandidate;
      }
      pW = pW->parentWidget();
    }
    if (pRootDockW) {
      // if that oldest ancestor is not already in the list, append it
      bool found = false;
      QPtrListIterator<KDockWidget> it2( *pRootDockWidgetList);
      if (!pRootDockWidgetList->isEmpty()) {
        for ( ; it2.current() && !found; ++it2 ) {
          KDockWidget* pDockW = it2.current();
          if (pDockW == pRootDockW)
            found = true;
        }
        if (!found) {
          pRootDockWidgetList->append( (KDockWidget*)pDockW);
//          kdDebug(760)<<"pRootDockWidgetList->append("<<pDockW->name()<<");"<<endl;
          QPoint p = pDockW->mapToGlobal( pDockW->pos())-pDockW->pos();
          QRect r( p.x(),
              p.y()+m_undockPositioningOffset.y(),
              pDockW->width()  - windowTitleHeight - frameBorderWidth*2,
              pDockW->height() - windowTitleHeight - frameBorderWidth*2);
          pPositionList->append( r);
        }
      }
      else {
        pRootDockWidgetList->append( (KDockWidget*)pRootDockW);
//        kdDebug(760)<<"pRootDockWidgetList->append("<<pDockW->name()<<");"<<endl;
        QPoint p = pRootDockW->mapToGlobal( pRootDockW->pos())-pRootDockW->pos();
        QRect r( p.x(),
            p.y()+m_undockPositioningOffset.y(),
            pRootDockW->width()  - windowTitleHeight - frameBorderWidth*2,
            pRootDockW->height() - windowTitleHeight - frameBorderWidth*2);
        pPositionList->append( r);
      }
    }
  }
  delete pObjList;
}

/**
 * undocks all view windows (unix-like)
 */
void KMdiMainFrm::switchToToplevelMode()
{
  if (m_mdiMode == KMdi::ToplevelMode) {
    emit mdiModeHasBeenChangedTo(KMdi::ToplevelMode);
    return;
  }

  KMdi::MdiMode oldMdiMode = m_mdiMode;

  const int frameBorderWidth  = 7;  // @todo: Can we / do we need to ask the window manager?
  setUndockPositioningOffset( QPoint( 0, (m_pTaskBar ? m_pTaskBar->height() : 0) + frameBorderWidth));


  // 1.) select the dockwidgets to be undocked and store their geometry
  QPtrList<KDockWidget> rootDockWidgetList;
  QValueList<QRect> positionList;
  if (oldMdiMode!=KMdi::IDEAlMode)
    findRootDockWidgets(&rootDockWidgetList, &positionList);

  // 2.) undock the MDI views of QextMDI
  if (oldMdiMode == KMdi::ChildframeMode) {
    finishChildframeMode();
  } else if (oldMdiMode == KMdi::TabPageMode) { // if tabified, release all views from their docking covers
    finishTabPageMode();
  } else if (m_mdiMode == KMdi::IDEAlMode) {
    finishIDEAlMode();
    findRootDockWidgets(&rootDockWidgetList, &positionList);
  }

  //   if (hasMenuBar()) menuBar()->setTopLevelMenu(false);

  // 3.) undock all these found oldest ancestors (being KDockWidgets)
  QPtrListIterator<KDockWidget> it3( rootDockWidgetList);
  for (; it3.current(); ++it3 ) {
    KDockWidget* pDockW = it3.current();
    pDockW->undock();
  }

  // 4.) recreate the MDI childframe area and hide it
  if ((oldMdiMode == KMdi::TabPageMode) || (oldMdiMode==KMdi::IDEAlMode)) {
    if (!m_pDockbaseAreaOfDocumentViews) {
      m_pDockbaseAreaOfDocumentViews = createDockWidget( "mdiAreaCover", QPixmap(), 0L, "mdi_area_cover");
      m_pDockbaseAreaOfDocumentViews->setDockWindowTransient(this,true);
      m_pDockbaseAreaOfDocumentViews->setEnableDocking(KDockWidget::DockNone);
      m_pDockbaseAreaOfDocumentViews->setDockSite(KDockWidget::DockCorner);
      m_pDockbaseAreaOfDocumentViews->setWidget(m_pMdi);
    }
    // set this dock to main view
    setView(m_pDockbaseAreaOfDocumentViews);
    setMainDockWidget(m_pDockbaseAreaOfDocumentViews);
    //REMOVE      m_pDockbaseOfTabPage = m_pDockbaseAreaOfDocumentViews;
  }
  QApplication::sendPostedEvents();
  if (!parentWidget()) {
    m_oldMainFrmMinHeight = minimumHeight();
    m_oldMainFrmMaxHeight = maximumHeight();
    m_oldMainFrmHeight = height();
    if( m_pDocumentViews->count())
      setFixedHeight( height() - m_pDockbaseAreaOfDocumentViews->height());
    else { // consider space for the taskbar
      QApplication::sendPostedEvents();
      setFixedHeight( height() - m_pDockbaseAreaOfDocumentViews->height() + 27);
    }
  }

#ifdef __GNUC__
#warning fixme
#endif
  // 5. show the child views again
  QPtrListIterator<KMdiChildView> it( *m_pDocumentViews);
  for( it.toFirst(); it.current(); ++it) {
    KMdiChildView* pView = it.current();
#if defined Q_WS_X11 && ! defined K_WS_QTONLY
    XSetTransientForHint(qt_xdisplay(),pView->winId(),winId());
#endif
    //      if( !pView->isToolView())
    pView->show();
  }

  // 6.) reset all memorized positions of the undocked ones and show them again
  QValueList<QRect>::Iterator it5;
  for (it3.toFirst(), it5 = positionList.begin() ; it3.current(), it5 != positionList.end(); ++it3, ++it5 ) {
    KDockWidget* pDockW = it3.current();
    pDockW->setGeometry( (*it5));
    pDockW->show();
  }

  m_pDockbaseAreaOfDocumentViews->setDockSite(KDockWidget::DockNone);
  m_mdiMode = KMdi::ToplevelMode;
  //qDebug("ToplevelMode on");

  emit mdiModeHasBeenChangedTo(KMdi::ToplevelMode);

}

void KMdiMainFrm::finishToplevelMode()
{
  m_pDockbaseAreaOfDocumentViews->setDockSite(KDockWidget::DockCorner);
  //KIPC::sendMessage(KIPC::ToolbarStyleChanged,winId());
}

/**
 * docks all view windows (Windows-like)
 */
void KMdiMainFrm::switchToChildframeMode()
{
  if (m_mdiMode == KMdi::ChildframeMode) {
    emit mdiModeHasBeenChangedTo(KMdi::ChildframeMode);
    return;
  }

  QPtrList<KDockWidget> rootDockWidgetList;
  if (m_mdiMode == KMdi::TabPageMode) {
    // select the dockwidgets to be undocked and store their geometry
    QValueList<QRect> positionList;
    findRootDockWidgets(&rootDockWidgetList, &positionList);

    // undock all these found oldest ancestors (being KDockWidgets)
    QPtrListIterator<KDockWidget> it3( rootDockWidgetList);
    for (; it3.current(); ++it3 ) {
      KDockWidget* pDockW = it3.current();
      pDockW->undock();
    }

    finishTabPageMode();
  } else if (m_mdiMode == KMdi::ToplevelMode) {
    finishToplevelMode();
  } else if (m_mdiMode == KMdi::IDEAlMode) {
    finishIDEAlMode(false);

    // select the dockwidgets to be undocked and store their geometry
    QValueList<QRect> positionList;
    findRootDockWidgets(&rootDockWidgetList, &positionList);



    // undock all these found oldest ancestors (being KDockWidgets)

    QPtrListIterator<KDockWidget> it3( rootDockWidgetList);
    for (; it3.current(); ++it3 ) {
      KDockWidget* pDockW = it3.current();
      pDockW->undock();
    }
    m_mdiMode=KMdi::TabPageMode;
    finishTabPageMode();
    m_mdiMode=KMdi::IDEAlMode;

  }

  //return; //debug


  if (!m_pDockbaseAreaOfDocumentViews) {
    // cover KMdi's childarea by a dockwidget
    m_pDockbaseAreaOfDocumentViews = createDockWidget( "mdiAreaCover", QPixmap(), 0L, "mdi_area_cover");
    m_pDockbaseAreaOfDocumentViews->setDockWindowTransient(this,true);
    m_pDockbaseAreaOfDocumentViews->setEnableDocking(KDockWidget::DockNone);
    m_pDockbaseAreaOfDocumentViews->setDockSite(KDockWidget::DockCorner);
    m_pDockbaseAreaOfDocumentViews->setWidget(m_pMdi);
//    kdDebug(760)<<"!swtichToChildframeMode: m_pDockbaseAreaOfDocumentViews"<<endl;
  }
  if (m_pDockbaseAreaOfDocumentViews->isTopLevel()) {
    // set this dock to main view
    setView(m_pDockbaseAreaOfDocumentViews);
    setMainDockWidget(m_pDockbaseAreaOfDocumentViews);
    m_pDockbaseAreaOfDocumentViews->setEnableDocking(KDockWidget::DockNone);
    m_pDockbaseAreaOfDocumentViews->setDockSite(KDockWidget::DockCorner);
    //REMOVE      m_pDockbaseOfTabPage = m_pDockbaseAreaOfDocumentViews;
//    kdDebug(760)<<"swtichToChildframeMode: m_pDockbaaseAreaOfDocumentViews->isTopLevel()"<<endl;
  }
  m_pDockbaseAreaOfDocumentViews->setWidget(m_pMdi); //JW
  m_pDockbaseAreaOfDocumentViews->show();
  //return; //debug
  if ( (m_mdiMode == KMdi::TabPageMode) || (m_mdiMode == KMdi::IDEAlMode)) {
//    kdDebug(760)<<"switchToChildFrameMode: trying to dock back toolviews"<<endl;
    QPtrListIterator<KDockWidget> it4( rootDockWidgetList);
    for (; it4.current(); ++it4 ) {
      KDockWidget* pDockW = it4.current();
      pDockW->dockBack();
    }
  }

  if (m_mdiMode == KMdi::ToplevelMode && m_pTempDockSession) {
    // restore the old dock szenario which we memorized at the time we switched to toplevel mode
    QDomElement oldDockState = m_pTempDockSession->namedItem("cur_dock_state").toElement();
    readDockConfig( oldDockState);
  }

  KMdi::MdiMode oldMdiMode = m_mdiMode;
  m_mdiMode = KMdi::ChildframeMode;

#ifdef __GNUC__
#warning fixme
#endif
  QPtrListIterator<KMdiChildView> it( *m_pDocumentViews);
  for( ; it.current(); ++it) {
    KMdiChildView* pView = it.current();
    if( !pView->isToolView())
      if( !pView->isAttached())
        attachWindow( pView, true);
  }
  for( it.toFirst(); it.current(); ++it) {
    KMdiChildView* pView = it.current();
    if( !pView->isToolView())
      pView->show();
  }
  if( (oldMdiMode == KMdi::ToplevelMode) && !parentWidget()) {
    setMinimumHeight( m_oldMainFrmMinHeight);
    setMaximumHeight( m_oldMainFrmMaxHeight);
    resize( width(), m_oldMainFrmHeight);
    m_oldMainFrmHeight = 0;
    //qDebug("TopLevelMode off");
    emit leftTopLevelMode();
  }
  emit mdiModeHasBeenChangedTo(KMdi::ChildframeMode);
}

void KMdiMainFrm::finishChildframeMode()
{
  // save the old dock szenario of the dockwidged-like tool views to a DOM tree
  delete m_pTempDockSession;
  m_pTempDockSession = new QDomDocument( "docksession");
  QDomElement curDockState = m_pTempDockSession->createElement("cur_dock_state");
  m_pTempDockSession->appendChild( curDockState);
  writeDockConfig( curDockState);

  // detach all non-tool-views to toplevel
  QPtrListIterator<KMdiChildView> it( *m_pDocumentViews);
  for( ; it.current(); ++it) {
    KMdiChildView* pView = it.current();
    if( pView->isToolView())
      continue;
    if( pView->isAttached()) {
      if( pView->isMaximized())
        pView->mdiParent()->setGeometry( 0, 0, m_pMdi->width(), m_pMdi->height());
      detachWindow( pView, false);
    }
  }
}




/**
 * Docks all view windows (Windows-like)
 */
void KMdiMainFrm::switchToTabPageMode()
{
  KMdiChildView* pRemActiveWindow = activeWindow();

  if (m_mdiMode == KMdi::TabPageMode) {
    emit mdiModeHasBeenChangedTo(KMdi::TabPageMode);
    return;  // nothing need to be done
  }

  // make sure that all MDI views are detached
  if (m_mdiMode == KMdi::ChildframeMode) {
    finishChildframeMode();
  } else if (m_mdiMode == KMdi::ToplevelMode) {
    finishToplevelMode();
  } else if (m_mdiMode == KMdi::IDEAlMode) {
    finishIDEAlMode(false);
    emit mdiModeHasBeenChangedTo(KMdi::TabPageMode);
    m_mdiMode=KMdi::TabPageMode;
    return;
  }

  setupTabbedDocumentViewSpace();
  m_mdiMode = KMdi::TabPageMode;
  if (pRemActiveWindow)
    pRemActiveWindow->setFocus();

  m_pTaskBar->switchOn(false);

  assert(m_pClose);
  QObject::connect( m_pClose, SIGNAL(clicked()), this, SLOT(closeViewButtonPressed()) );
  if (m_pDocumentViews->count() > 0) {
    m_pClose->show();
  }
  //qDebug("TabPageMode on");
  emit mdiModeHasBeenChangedTo(KMdi::TabPageMode);
}

void KMdiMainFrm::finishTabPageMode()
{
  // if tabified, release all views from their docking covers
  if (m_mdiMode == KMdi::TabPageMode) {
    m_pClose->hide();
    QObject::disconnect( m_pClose, SIGNAL(clicked()), this, SLOT(closeViewButtonPressed()) );

    QPtrListIterator<KMdiChildView> it( *m_pDocumentViews);
    for( ; it.current(); ++it) {
      KMdiChildView* pView = it.current();
      if( pView->isToolView())
        continue;
//      kdDebug(760)<<"KMdiMainFrm::finishTabPageMode: in loop"<<endl;
      QSize mins = pView->minimumSize();
      QSize maxs = pView->maximumSize();
      QSize sz = pView->size();
      QWidget* pParent = pView->parentWidget();
      QPoint p(pParent->mapToGlobal(pParent->pos())-pParent->pos()+m_undockPositioningOffset);
      m_documentTabWidget->removePage(pView);
      pView->reparent(0,0,p);
      //         pView->reparent(0,0,p);
      pView->resize(sz);
      pView->setMinimumSize(mins.width(),mins.height());
      pView->setMaximumSize(maxs.width(),maxs.height());
      //         ((KDockWidget*)pParent)->undock(); // this destroys the dockwiget cover, too
      //         pParent->close();
      //         delete pParent;
      //         if (centralWidget() == pParent) {
      //            setCentralWidget(0L); // avoid dangling pointer
      //         }
    }
    delete m_documentTabWidget;
    m_documentTabWidget=0;
    m_pTaskBar->switchOn(true);
  }
}



void KMdiMainFrm::setupTabbedDocumentViewSpace() {
  // resize to childframe mode size of the mainwindow if we were in toplevel mode
  if( (m_mdiMode == KMdi::ToplevelMode) && !parentWidget()) {
    setMinimumHeight( m_oldMainFrmMinHeight);
    setMaximumHeight( m_oldMainFrmMaxHeight);
    resize( width(), m_oldMainFrmHeight);
    m_oldMainFrmHeight = 0;
    //qDebug("TopLevelMode off");
    emit leftTopLevelMode();
    QApplication::sendPostedEvents();

    // restore the old dock szenario which we memorized at the time we switched to toplevel mode
    if (m_pTempDockSession) {
      QDomElement oldDockState = m_pTempDockSession->namedItem("cur_dock_state").toElement();
      readDockConfig( oldDockState);
    }
  }

#if 0
  if (m_pDockbaseOfTabPage != m_pDockbaseAreaOfDocumentViews) {
    delete m_pDockbaseOfTabPage;
    m_pDockbaseOfTabPage = m_pDockbaseAreaOfDocumentViews;
  }
#endif
  delete m_documentTabWidget;
  m_documentTabWidget=new KMdiDocumentViewTabWidget(m_pDockbaseAreaOfDocumentViews);
  connect(m_documentTabWidget,SIGNAL(currentChanged(QWidget*)),this,SLOT(slotDocCurrentChanged(QWidget*)));
  m_pDockbaseAreaOfDocumentViews->setWidget(m_documentTabWidget);
  m_documentTabWidget->show();
  QPtrListIterator<KMdiChildView> it4( *m_pDocumentViews);
  for( ; it4.current(); ++it4) {
    KMdiChildView* pView = it4.current();
    m_documentTabWidget->addTab(pView, pView->icon() ? *(pView->icon()) : QPixmap(),pView->tabCaption());
    /*
       connect(pView,SIGNAL(iconOrCaptionUdpated(QWidget*,QPixmap,const QString&)),
       m_documentTabWidget,SLOT(updateView(QWidget*,QPixmap,const QString&)));
     */
    connect( pView, SIGNAL(iconUpdated(QWidget*, QPixmap )), m_documentTabWidget, SLOT(updateIconInView(QWidget*, QPixmap )) );
    connect( pView, SIGNAL(captionUpdated(QWidget*, const QString& )), m_documentTabWidget, SLOT(updateCaptionInView(QWidget*, const QString& )) );

  }
}


void KMdiMainFrm::setIDEAlModeStyle(int flags)
{
  d->m_styleIDEAlMode = flags; // see KMultiTabBar for the first 3 bits
  if (m_leftContainer) {
    KMdiDockContainer *tmpL=(KMdiDockContainer*) (m_leftContainer->getWidget()->qt_cast("KMdiDockContainer"));
    if (tmpL) tmpL->setStyle(flags);
  }

  if (m_rightContainer) {
    KMdiDockContainer *tmpR=(KMdiDockContainer*) (m_rightContainer->getWidget()->qt_cast("KMdiDockContainer"));
    if (tmpR) tmpR->setStyle(flags);
  }

  if (m_topContainer) {
    KMdiDockContainer *tmpT=(KMdiDockContainer*) (m_topContainer->getWidget()->qt_cast("KMdiDockContainer"));
    if (tmpT) tmpT->setStyle(flags);
  }

  if (m_bottomContainer) {
    KMdiDockContainer *tmpB=(KMdiDockContainer*) (m_bottomContainer->getWidget()->qt_cast("KMdiDockContainer"));
    if (tmpB) tmpB->setStyle(flags);
  }
}

void KMdiMainFrm::setToolviewStyle(int flag)
{
  if (m_mdiMode == KMdi::IDEAlMode) {
    setIDEAlModeStyle(flag);
  }
  d->m_toolviewStyle = flag;
  bool toolviewExists = false;
  QMap<QWidget*,KMdiToolViewAccessor*>::Iterator it;
  for (it = m_pToolViews->begin(); it != m_pToolViews->end(); ++it) {
    KDockWidget *dockWidget = dynamic_cast<KDockWidget*>(it.data()->wrapperWidget());
    if (dockWidget) {
      if (flag == KMdi::IconOnly)
      {
        dockWidget->setTabPageLabel(" ");
        dockWidget->setPixmap(*(it.data()->wrappedWidget()->icon()));
      } else
      if (flag == KMdi::TextOnly)
      {
        dockWidget->setPixmap(); //FIXME: Does not hide the icon in the IDEAl mode.
        dockWidget->setTabPageLabel(it.data()->wrappedWidget()->caption());
      } else
      if (flag == KMdi::TextAndIcon)
      {
        dockWidget->setPixmap(*(it.data()->wrappedWidget()->icon()));
        dockWidget->setTabPageLabel(it.data()->wrappedWidget()->caption());
      }
      toolviewExists = true;
    }
  }
  if (toolviewExists)
  {
    //workaround for the above FIXME to make switching to TextOnly mode work in IDEAl as well. Be sure that this version of switch* is called.
    if (m_mdiMode == KMdi::IDEAlMode && flag == KMdi::TextOnly)
    {
      KMdiMainFrm::switchToTabPageMode();
      KMdiMainFrm::switchToIDEAlMode();
    } else
    {
      writeDockConfig();
      readDockConfig();
    }
  }
}

/**
 * Docks all view windows (Windows-like)
 */
void KMdiMainFrm::switchToIDEAlMode()
{
//  kdDebug(760)<<"SWITCHING TO IDEAL"<<endl;
  KMdiChildView* pRemActiveWindow = activeWindow();

  if (m_mdiMode == KMdi::IDEAlMode) {
    emit mdiModeHasBeenChangedTo(KMdi::IDEAlMode);
    return;  // nothing need to be done
  }

  // make sure that all MDI views are detached
  if (m_mdiMode == KMdi::ChildframeMode) {
    finishChildframeMode();
  } else if (m_mdiMode == KMdi::ToplevelMode) {
    finishToplevelMode();
  } else if (m_mdiMode == KMdi::TabPageMode) {
    m_mdiMode=KMdi::IDEAlMode;
    setupToolViewsForIDEALMode();
    emit mdiModeHasBeenChangedTo(KMdi::IDEAlMode);
    return;
  }

  setupTabbedDocumentViewSpace();
  m_mdiMode = KMdi::IDEAlMode;


  setupToolViewsForIDEALMode();

  if (pRemActiveWindow)
    pRemActiveWindow->setFocus();

  m_pTaskBar->switchOn(false);

  assert(m_pClose);
  QObject::connect( m_pClose, SIGNAL(clicked()), this, SLOT(closeViewButtonPressed()) );
  if (m_pDocumentViews->count() > 0) {
    m_pClose->show();
  }
  //qDebug("IDEAlMode on");

  emit mdiModeHasBeenChangedTo(KMdi::IDEAlMode);
}


void KMdiMainFrm::dockToolViewsIntoContainers(QPtrList<KDockWidget>& widgetsToReparent,KDockWidget *container) {
  for ( KDockWidget *dw = widgetsToReparent.first(); dw;
      dw=widgetsToReparent.next()){
    dw->manualDock(container,KDockWidget::DockCenter,20);
    dw->loseFormerBrotherDockWidget();
  }
}

void KMdiMainFrm::findToolViewsDockedToMain(QPtrList<KDockWidget>* list,KDockWidget::DockPosition dprtmw) {
  KDockWidget *mainDock=getMainDockWidget();
  if (mainDock->parentDockTabGroup()) {
    mainDock=dynamic_cast<KDockWidget*>(mainDock->parentDockTabGroup()->parent());
    // FIXME: will likely crash below due to unchecked cast
  }

  KDockWidget* widget=mainDock->findNearestDockWidget(dprtmw);
  if (widget) {
    if (widget->parentDockTabGroup()) {
      widget=static_cast<KDockWidget*>(widget->parentDockTabGroup()->parent());
    }

    if (widget) {
      KDockTabGroup *tg=dynamic_cast<KDockTabGroup*>(widget->
          getWidget());
      if (tg) {
//        kdDebug(760)<<"KDockTabGroup found"<<endl;
        for (int i=0;i<tg->count();i++)
          list->append((KDockWidget*)static_cast<KDockWidget*>(
                tg->page(i)));
      } else
        list->append((KDockWidget*)widget);
    }// else
//      kdDebug(760)<<"setupToolViewsForIDEALMode: no  widget found"<<endl;
  } //else
//    kdDebug(760)<<"No main dock widget found"<<endl;
}


void KMdiMainFrm::setupToolViewsForIDEALMode()
{
  m_leftContainer = createDockWidget("KMdiDock::leftDock",SmallIcon("misc"),0L,"Left Dock");
  m_rightContainer = createDockWidget("KMdiDock::rightDock",SmallIcon("misc"),0L,"Right Dock");
  m_topContainer = createDockWidget("KMdiDock::topDock",SmallIcon("misc"),0L,"Top Dock");
  m_bottomContainer = createDockWidget("KMdiDock::bottomDock",SmallIcon("misc"),0L,"Bottom Dock");

  KDockWidget *mainDock=getMainDockWidget();
  KDockWidget *w=mainDock;
  if (mainDock->parentDockTabGroup()) {
    w=static_cast<KDockWidget*>(mainDock->parentDockTabGroup()->parent());
  }

  QPtrList<KDockWidget> leftReparentWidgets;
  QPtrList<KDockWidget> rightReparentWidgets;
  QPtrList<KDockWidget> bottomReparentWidgets;
  QPtrList<KDockWidget> topReparentWidgets;

  if (mainDock->parentDockTabGroup()) {
    mainDock=static_cast<KDockWidget*>(mainDock->parentDockTabGroup()->parent());
  }

  findToolViewsDockedToMain(&leftReparentWidgets,KDockWidget::DockLeft);
  findToolViewsDockedToMain(&rightReparentWidgets,KDockWidget::DockRight);
  findToolViewsDockedToMain(&bottomReparentWidgets,KDockWidget::DockBottom);
  findToolViewsDockedToMain(&topReparentWidgets,KDockWidget::DockTop);

  mainDock->setEnableDocking(KDockWidget::DockNone); //::DockCorner);
  mainDock->setDockSite(KDockWidget::DockCorner);


  KMdiDockContainer *tmpDC;
  m_leftContainer->setWidget(tmpDC=new KMdiDockContainer(m_leftContainer, this, KDockWidget::DockLeft, d->m_styleIDEAlMode));
  m_leftContainer->setEnableDocking(KDockWidget::DockLeft);
  m_leftContainer->manualDock(mainDock, KDockWidget::DockLeft,20);
  tmpDC->init();
  if (m_mdiGUIClient) connect (this,SIGNAL(toggleLeft()),tmpDC,SLOT(toggle()));
  connect(this,SIGNAL(collapseOverlapContainers()),tmpDC,SLOT(collapseOverlapped()));
  connect(tmpDC,SIGNAL(activated(KMdiDockContainer*)),this,SLOT(setActiveToolDock(KMdiDockContainer*)));
  connect(tmpDC,SIGNAL(deactivated(KMdiDockContainer*)),this,SLOT(removeFromActiveDockList(KMdiDockContainer*)));

  m_rightContainer->setWidget(tmpDC=new KMdiDockContainer(m_rightContainer, this, KDockWidget::DockRight, d->m_styleIDEAlMode));
  m_rightContainer->setEnableDocking(KDockWidget::DockRight);
  m_rightContainer->manualDock(mainDock, KDockWidget::DockRight,80);
  tmpDC->init();
  if (m_mdiGUIClient) connect (this,SIGNAL(toggleRight()),tmpDC,SLOT(toggle()));
  connect(this,SIGNAL(collapseOverlapContainers()),tmpDC,SLOT(collapseOverlapped()));
  connect(tmpDC,SIGNAL(activated(KMdiDockContainer*)),this,SLOT(setActiveToolDock(KMdiDockContainer*)));
  connect(tmpDC,SIGNAL(deactivated(KMdiDockContainer*)),this,SLOT(removeFromActiveDockList(KMdiDockContainer*)));

  m_topContainer->setWidget(tmpDC=new KMdiDockContainer(m_topContainer, this, KDockWidget::DockTop, d->m_styleIDEAlMode));
  m_topContainer->setEnableDocking(KDockWidget::DockTop);
  m_topContainer->manualDock(mainDock, KDockWidget::DockTop,20);
  tmpDC->init();
  if (m_mdiGUIClient) connect (this,SIGNAL(toggleTop()),tmpDC,SLOT(toggle()));
  connect(this,SIGNAL(collapseOverlapContainers()),tmpDC,SLOT(collapseOverlapped()));
  connect(tmpDC,SIGNAL(activated(KMdiDockContainer*)),this,SLOT(setActiveToolDock(KMdiDockContainer*)));
  connect(tmpDC,SIGNAL(deactivated(KMdiDockContainer*)),this,SLOT(removeFromActiveDockList(KMdiDockContainer*)));

  m_bottomContainer->setWidget(tmpDC=new KMdiDockContainer(m_bottomContainer, this, KDockWidget::DockBottom, d->m_styleIDEAlMode));
  m_bottomContainer->setEnableDocking(KDockWidget::DockBottom);
  m_bottomContainer->manualDock(mainDock, KDockWidget::DockBottom,80);
  tmpDC->init();
  if (m_mdiGUIClient) connect (this,SIGNAL(toggleBottom()),tmpDC,SLOT(toggle()));
  connect(this,SIGNAL(collapseOverlapContainers()),tmpDC,SLOT(collapseOverlapped()));
  connect(tmpDC,SIGNAL(activated(KMdiDockContainer*)),this,SLOT(setActiveToolDock(KMdiDockContainer*)));
  connect(tmpDC,SIGNAL(deactivated(KMdiDockContainer*)),this,SLOT(removeFromActiveDockList(KMdiDockContainer*)));

  m_leftContainer->setDockSite( KDockWidget::DockCenter );
  m_rightContainer->setDockSite( KDockWidget::DockCenter );
  m_topContainer->setDockSite( KDockWidget::DockCenter );
  m_bottomContainer->setDockSite( KDockWidget::DockCenter );

  dockToolViewsIntoContainers(leftReparentWidgets,m_leftContainer);
  dockToolViewsIntoContainers(rightReparentWidgets,m_rightContainer);
  dockToolViewsIntoContainers(bottomReparentWidgets,m_bottomContainer);
  dockToolViewsIntoContainers(topReparentWidgets,m_topContainer);


  dockManager->setSpecialLeftDockContainer(m_leftContainer);
  dockManager->setSpecialRightDockContainer(m_rightContainer);
  dockManager->setSpecialTopDockContainer(m_topContainer);
  dockManager->setSpecialBottomDockContainer(m_bottomContainer);


  ((KMdiDockContainer*) (m_leftContainer->getWidget()))->hideIfNeeded();
  ((KMdiDockContainer*) (m_rightContainer->getWidget()))->hideIfNeeded();
  ((KMdiDockContainer*) (m_topContainer->getWidget()))->hideIfNeeded();
  ((KMdiDockContainer*) (m_bottomContainer->getWidget()))->hideIfNeeded();

}



void KMdiMainFrm::finishIDEAlMode(bool full)
{
  // if tabified, release all views from their docking covers
  if (m_mdiMode == KMdi::IDEAlMode) {
    assert(m_pClose);
    m_pClose->hide();
    QObject::disconnect( m_pClose, SIGNAL(clicked()), this, SLOT(closeViewButtonPressed()) );


    QStringList leftNames;
    leftNames=prepareIdealToTabs(m_leftContainer);
    int leftWidth=m_leftContainer->width();

    QStringList rightNames;
    rightNames=prepareIdealToTabs(m_rightContainer);
    int rightWidth=m_rightContainer->width();

    QStringList topNames;
    topNames=prepareIdealToTabs(m_topContainer);
    int topHeight=m_topContainer->height();

    QStringList bottomNames;
    bottomNames=prepareIdealToTabs(m_bottomContainer);
    int bottomHeight=m_bottomContainer->height();


//    kdDebug(760)<<"leftNames"<<leftNames<<endl;
//    kdDebug(760)<<"rightNames"<<rightNames<<endl;
//    kdDebug(760)<<"topNames"<<topNames<<endl;
//    kdDebug(760)<<"bottomNames"<<bottomNames<<endl;

    delete m_leftContainer;
    m_leftContainer=0;
    delete m_rightContainer;
    m_rightContainer=0;
    delete m_bottomContainer;
    m_bottomContainer=0;
    delete m_topContainer;
    m_topContainer=0;


    idealToolViewsToStandardTabs(bottomNames,KDockWidget::DockBottom,bottomHeight);
    idealToolViewsToStandardTabs(leftNames,KDockWidget::DockLeft,leftWidth);
    idealToolViewsToStandardTabs(rightNames,KDockWidget::DockRight,rightWidth);
    idealToolViewsToStandardTabs(topNames,KDockWidget::DockTop,topHeight);

    QApplication::sendPostedEvents();

    if (!full) return;

    QPtrListIterator<KMdiChildView> it( *m_pDocumentViews);
    for( ; it.current(); ++it) {
      KMdiChildView* pView = it.current();
      if( pView->isToolView())
        continue;
      QSize mins = pView->minimumSize();
      QSize maxs = pView->maximumSize();
      QSize sz = pView->size();
      QWidget* pParent = pView->parentWidget();
      QPoint p(pParent->mapToGlobal(pParent->pos())-pParent->pos()+m_undockPositioningOffset);
      pView->reparent(0,0,p);
      pView->reparent(0,0,p);
      pView->resize(sz);
      pView->setMinimumSize(mins.width(),mins.height());
      pView->setMaximumSize(maxs.width(),maxs.height());
      KDockWidget* pDockW = 0L;
      // find the oldest ancestor of the current dockwidget that can be undocked
      do {
        if (pParent->inherits("KDockWidget") || pParent->inherits("KDockWidget_Compat::KDockWidget")) {
          pDockW = (KDockWidget*) pParent;
          pDockW->undock(); // this destroys the dockwiget cover, too
          if (pParent != m_pDockbaseAreaOfDocumentViews) {
            pParent->close();
            delete pParent;
          }
        }
        else {
          pParent = pParent->parentWidget();
        }
      }
      while (pParent && !pDockW);
      if (centralWidget() == pParent) {
        setCentralWidget(0L); // avoid dangling pointer
      }
    }
    m_pTaskBar->switchOn(true);

  }

}

QStringList KMdiMainFrm::prepareIdealToTabs(KDockWidget* container) {
  KDockContainer* pDW = dynamic_cast<KDockContainer*>(container->getWidget());
  QStringList widgetNames=((KMdiDockContainer*)pDW)->containedWidgets();
  for (QStringList::iterator it=widgetNames.begin();it!=widgetNames.end();++it) {
    KDockWidget* dw = (KDockWidget*) manager()->getDockWidgetFromName(*it);
    dw->undock();
    dw->setLatestKDockContainer(0);
    dw->loseFormerBrotherDockWidget();
  }
  return widgetNames;
}

void KMdiMainFrm::idealToolViewsToStandardTabs(QStringList widgetNames,KDockWidget::DockPosition pos,int size) {
  Q_UNUSED(size)

  KDockWidget *mainDock=getMainDockWidget();
  if (mainDock->parentDockTabGroup()) {
    mainDock=static_cast<KDockWidget*>(mainDock->parentDockTabGroup()->parent());
  }

  if(widgetNames.count()>0) {
    QStringList::iterator it=widgetNames.begin();
    KDockWidget *dwpd=manager()->getDockWidgetFromName(*it);
    if (!dwpd) {
//      kdDebug(760)<<"Fatal error in finishIDEAlMode"<<endl;
      return;
    }
    dwpd->manualDock(mainDock,pos,20);
    ++it;
    for (;it!=widgetNames.end();++it) {
      KDockWidget *tmpdw=manager()->getDockWidgetFromName(*it);
      if (!tmpdw) {
//        kdDebug(760)<<"Fatal error in finishIDEAlMode"<<endl;
        return;
      }
      tmpdw->manualDock(dwpd,KDockWidget::DockCenter,20);
    }

#if 0
    QWidget *wid=dwpd->parentDockTabGroup();
    if (!wid) wid=dwpd;
    wid->setGeometry(0,0,20,20);
    /*  wid->resize(
        ((pos==KDockWidget::DockLeft) || (pos==KDockWidget::DockRight))?size:wid->width(),
        ((pos==KDockWidget::DockLeft) || (pos==KDockWidget::DockRight))?wid->height():size);
     */
#endif
  }

}


/**
 * redirect the signal for insertion of buttons to an own slot
 * that means: If the menubar (where the buttons should be inserted) is given,
 *             QextMDI can insert them automatically.
 *             Otherwise only signals can be emitted to tell the outside that
 *             someone must do this job itself.
 */
void KMdiMainFrm::setMenuForSDIModeSysButtons( KMenuBar* pMenuBar)
{
  if (m_bSDIApplication)  // there are no buttons in the menubar in this mode (although the view is always maximized)
    return;

  m_pMainMenuBar = pMenuBar;
  if( m_pMainMenuBar == 0L)
    return;  // use setMenuForSDIModeSysButtons( 0L) for unsetting the external main menu!

  if (!m_pUndock)
    m_pUndock = new QToolButton( pMenuBar);
  if (!m_pRestore)
    m_pRestore = new QToolButton( pMenuBar);
  if (!m_pMinimize)
    m_pMinimize = new QToolButton( pMenuBar);
  if (!m_pClose)
    m_pClose = new QToolButton( pMenuBar);
  m_pUndock->setAutoRaise(false);
  m_pMinimize->setAutoRaise(false);
  m_pRestore->setAutoRaise(false);
  m_pClose->setAutoRaise(false);

  setSysButtonsAtMenuPosition();

  delete m_pUndockButtonPixmap;
  delete m_pMinButtonPixmap;
  delete m_pRestoreButtonPixmap;
  delete m_pCloseButtonPixmap;
  // create the decoration pixmaps
  if (frameDecorOfAttachedViews() == KMdi::Win95Look) {
    m_pUndockButtonPixmap = new QPixmap( win_undockbutton);
    m_pMinButtonPixmap = new QPixmap( win_minbutton);
    m_pRestoreButtonPixmap = new QPixmap( win_restorebutton);
    m_pCloseButtonPixmap = new QPixmap( win_closebutton);
  }
  else if (frameDecorOfAttachedViews() == KMdi::KDE1Look) {
    m_pUndockButtonPixmap = new QPixmap( kde_undockbutton);
    m_pMinButtonPixmap = new QPixmap( kde_minbutton);
    m_pRestoreButtonPixmap = new QPixmap( kde_restorebutton);
    m_pCloseButtonPixmap = new QPixmap( kde_closebutton);
    m_pUndock->setAutoRaise(true);
    m_pMinimize->setAutoRaise(true);
    m_pRestore->setAutoRaise(true);
    m_pClose->setAutoRaise(true);
  }
  else if (frameDecorOfAttachedViews() == KMdi::KDELook) {
    m_pUndockButtonPixmap = new QPixmap( kde2_undockbutton);
    m_pMinButtonPixmap = new QPixmap( kde2_minbutton);
    m_pRestoreButtonPixmap = new QPixmap( kde2_restorebutton);
    m_pCloseButtonPixmap = new QPixmap( kde2_closebutton);
  }
  else {   // kde2laptop look
    m_pUndockButtonPixmap = new QPixmap( kde2laptop_undockbutton);
    m_pMinButtonPixmap = new QPixmap( kde2laptop_minbutton);
    m_pRestoreButtonPixmap = new QPixmap( kde2laptop_restorebutton);
    m_pCloseButtonPixmap = new QPixmap( kde2laptop_closebutton);
  }

  m_pUndock->hide();
  m_pMinimize->hide();
  m_pRestore->hide();
  m_pClose->hide();

  m_pUndock->setPixmap( *m_pUndockButtonPixmap);
  m_pMinimize->setPixmap( *m_pMinButtonPixmap);
  m_pRestore->setPixmap( *m_pRestoreButtonPixmap);
  m_pClose->setPixmap( *m_pCloseButtonPixmap);
}

void KMdiMainFrm::setSysButtonsAtMenuPosition()
{
  if( m_pMainMenuBar == 0L)
    return;
  if( m_pMainMenuBar->parentWidget() == 0L)
    return;

  int menuW = m_pMainMenuBar->parentWidget()->width();
  int h;
  int y;
  if (frameDecorOfAttachedViews() == KMdi::Win95Look)
    h = 16;
  else if (frameDecorOfAttachedViews() == KMdi::KDE1Look)
    h = 20;
  else if (frameDecorOfAttachedViews() == KMdi::KDELook)
    h = 16;
  else
    h = 14;
  y = m_pMainMenuBar->height()/2 - h/2;

  if (frameDecorOfAttachedViews() == KMdi::KDELaptopLook) {
    int w = 27;
    m_pUndock->setGeometry( ( menuW - ( w * 3) - 5), y, w, h);
    m_pMinimize->setGeometry( ( menuW - ( w * 2) - 5), y, w, h);
    m_pRestore->setGeometry( ( menuW - w - 5), y, w, h);
  }
  else {
    m_pUndock->setGeometry( ( menuW - ( h * 4) - 5), y, h, h);
    m_pMinimize->setGeometry( ( menuW - ( h * 3) - 5), y, h, h);
    m_pRestore->setGeometry( ( menuW - ( h * 2) - 5), y, h, h);
    m_pClose->setGeometry( ( menuW - h - 5), y, h, h);
  }
}

/** Activates the next open view */
void KMdiMainFrm::activateNextWin()
{
  KMdiIterator<KMdiChildView*>* it = createIterator();
  KMdiChildView* aWin = activeWindow();
  for (it->first(); !it->isDone(); it->next()) {
    if (it->currentItem() == aWin) {
      it->next();
      if (!it->currentItem()) {
        it->first();
      }
      if (it->currentItem()) {
        activateView(it->currentItem());
      }
      break;
    }
  }
  delete it;
}

/** Activates the previous open view */
void KMdiMainFrm::activatePrevWin()
{
  KMdiIterator<KMdiChildView*>* it = createIterator();
  KMdiChildView* aWin = activeWindow();
  for (it->first(); !it->isDone(); it->next()) {
    if (it->currentItem() == aWin) {
      it->prev();
      if (!it->currentItem()) {
        it->last();
      }
      if (it->currentItem()) {
        activateView(it->currentItem());
      }
      break;
    }
  }
  delete it;
}

/** Activates the view we accessed the most time ago */
void KMdiMainFrm::activateFirstWin()
{
  KMdiIterator<KMdiChildView*>* it = createIterator();
  QMap<QDateTime,KMdiChildView*> m;
  for (it->first(); !it->isDone(); it->next()) {
    m.insert(it->currentItem()->getTimeStamp(), it->currentItem());
  }

  if ( !activeWindow() ) return;

  QDateTime current = activeWindow()->getTimeStamp();
  QMap<QDateTime,KMdiChildView*>::iterator pos(m.find(current));
  QMap<QDateTime,KMdiChildView*>::iterator newPos = pos;
  if (pos != m.end()) {
    ++newPos;
  }
  if (newPos != m.end()) { // look ahead
    ++pos;
  }
  else {
    pos = m.begin();
  }
  activateView(pos.data());
  m_bSwitching= true; // flag that we are currently switching between windows
  delete it;
}

/** Activates the previously accessed view before this one was activated */
void KMdiMainFrm::activateLastWin()
{
  KMdiIterator<KMdiChildView*>* it = createIterator();
  QMap<QDateTime,KMdiChildView*> m;
  for (it->first(); !it->isDone(); it->next()) {
    m.insert(it->currentItem()->getTimeStamp(), it->currentItem());
  }

  if ( !activeWindow() ) return;

  QDateTime current = activeWindow()->getTimeStamp();
  QMap<QDateTime,KMdiChildView*>::iterator pos(m.find(current));
  if (pos != m.begin()) {
    --pos;
  }
  else {
    pos = m.end();
    --pos;
  }
  activateView(pos.data());
  m_bSwitching= true; // flag that we are currently switching between windows
  delete it;
}

/** Activates the view with a certain index (TabPage mode only) */
void KMdiMainFrm::activateView(int index)
{
  KMdiChildView* pView = m_pDocumentViews->first();
  for (int i = 0; pView && (i < index); i++) {
    pView = m_pDocumentViews->next();
  }
  if (pView) {
    pView->activate();
  }
}

/** turns the system buttons for maximize mode (SDI mode) on, and connects them with the current child frame */
void KMdiMainFrm::setEnableMaximizedChildFrmMode(bool bEnable)
{
  if (bEnable) {
    m_bMaximizedChildFrmMode = true;
    //qDebug("MaximizeMode on");

    KMdiChildFrm* pCurrentChild = m_pMdi->topChild();
    if( !pCurrentChild)
      return;

    // if there is no menubar given, those system buttons aren't possible
    if( m_pMainMenuBar == 0L)
      return;

    QObject::connect( m_pUndock, SIGNAL(clicked()), pCurrentChild, SLOT(undockPressed()) );
    m_pUndock->show();
    QObject::connect( m_pMinimize, SIGNAL(clicked()), pCurrentChild, SLOT(minimizePressed()) );
    m_pMinimize->show();
    QObject::connect( m_pRestore, SIGNAL(clicked()), pCurrentChild, SLOT(maximizePressed()) );
    m_pRestore->show();

    if (frameDecorOfAttachedViews() == KMdi::KDELaptopLook) {
      m_pMainMenuBar->insertItem( QPixmap(kde2laptop_closebutton_menu), m_pMdi->topChild(), SLOT(closePressed()), 0, -1, 0);
    }
    else {
      m_pMainMenuBar->insertItem( *pCurrentChild->icon(), pCurrentChild->systemMenu(), -1, 0);
      assert(m_pClose);
      QObject::connect( m_pClose, SIGNAL(clicked()), pCurrentChild, SLOT(closePressed()) );
      m_pClose->show();
    }
  }
  else {
    if (!m_bMaximizedChildFrmMode) return;  // already set, nothing to do

    m_bMaximizedChildFrmMode = false;
    //qDebug("MaximizeMode off");

    KMdiChildFrm* pFrmChild = m_pMdi->topChild();
    if (!pFrmChild) return;

    if (pFrmChild->m_pClient && pFrmChild->state() == KMdiChildFrm::Maximized) {
      pFrmChild->m_pClient->restore();
      switchOffMaximizeModeForMenu( pFrmChild);
    }
  }
}

/** turns the system buttons for maximize mode (SDI mode) off, and disconnects them */
void KMdiMainFrm::switchOffMaximizeModeForMenu(KMdiChildFrm* oldChild)
{
  //qDebug("switching off maximize mode for menu");

  // if there is no menubar given, those system buttons aren't possible
  if( m_pMainMenuBar == 0L)
    return;

  m_pMainMenuBar->removeItem( m_pMainMenuBar->idAt(0));

  if( oldChild) {
    assert(m_pClose);
    QObject::disconnect( m_pUndock, SIGNAL(clicked()), oldChild, SLOT(undockPressed()) );
    QObject::disconnect( m_pMinimize, SIGNAL(clicked()), oldChild, SLOT(minimizePressed()) );
    QObject::disconnect( m_pRestore, SIGNAL(clicked()), oldChild, SLOT(maximizePressed()) );
    QObject::disconnect( m_pClose, SIGNAL(clicked()), oldChild, SLOT(closePressed()) );
  }
  m_pUndock->hide();
  m_pMinimize->hide();
  m_pRestore->hide();
  m_pClose->hide();
}

/** reconnects the system buttons form maximize mode (SDI mode) with the new child frame */
void KMdiMainFrm::updateSysButtonConnections( KMdiChildFrm* oldChild, KMdiChildFrm* newChild)
{
  //qDebug("updateSysButtonConnections");
  // if there is no menubar given, those system buttons aren't possible
  if( m_pMainMenuBar == 0L)
    return;

  if (newChild) {
    if (frameDecorOfAttachedViews() == KMdi::KDELaptopLook) {
      m_pMainMenuBar->insertItem( QPixmap(kde2laptop_closebutton_menu), newChild, SLOT(closePressed()), 0, -1, 0);
    }
    else {
      m_pMainMenuBar->insertItem( *newChild->icon(), newChild->systemMenu(), -1, 0);
    }
  }
  if (oldChild) {
    m_pMainMenuBar->removeItem( m_pMainMenuBar->idAt(1));
  }
  if (oldChild) {
    assert(m_pClose);
    QObject::disconnect( m_pUndock, SIGNAL(clicked()), oldChild, SLOT(undockPressed()) );
    QObject::disconnect( m_pMinimize, SIGNAL(clicked()), oldChild, SLOT(minimizePressed()) );
    QObject::disconnect( m_pRestore, SIGNAL(clicked()), oldChild, SLOT(maximizePressed()) );
    QObject::disconnect( m_pClose, SIGNAL(clicked()), oldChild, SLOT(closePressed()) );
  }
  if (newChild) {
    assert(m_pClose);
    QObject::connect( m_pUndock, SIGNAL(clicked()), newChild, SLOT(undockPressed()) );
    QObject::connect( m_pMinimize, SIGNAL(clicked()), newChild, SLOT(minimizePressed()) );
    QObject::connect( m_pRestore, SIGNAL(clicked()), newChild, SLOT(maximizePressed()) );
    QObject::connect( m_pClose, SIGNAL(clicked()), newChild, SLOT(closePressed()) );
  }
}

/** Shows the view taskbar. This should be connected with your "View" menu. */
bool KMdiMainFrm::isViewTaskBarOn()
{
  bool bOn = false;
  if (m_pTaskBar)
    bOn = m_pTaskBar->isSwitchedOn();
  return bOn;
}

/** Shows the view taskbar. This should be connected with your "View" menu. */
void KMdiMainFrm::showViewTaskBar()
{
  if (m_pTaskBar)
    m_pTaskBar->switchOn(true);
}

/** Hides the view taskbar. This should be connected with your "View" menu. */
void KMdiMainFrm::hideViewTaskBar()
{
  if (m_pTaskBar)
    m_pTaskBar->switchOn(false);
}

//=============== fillWindowMenu ===============//
void KMdiMainFrm::fillWindowMenu()
{
  bool bTabPageMode = false;
  if (m_mdiMode == KMdi::TabPageMode)
    bTabPageMode = true;
  bool bIDEAlMode = FALSE;
  if (m_mdiMode == KMdi::IDEAlMode)
    bIDEAlMode = TRUE;

  bool bNoViewOpened = false;
  if (m_pDocumentViews->isEmpty()) {
    bNoViewOpened = true;
  }
  // construct the menu and its submenus
  if (!m_bClearingOfWindowMenuBlocked) {
    m_pWindowMenu->clear();
  }
  int closeId = m_pWindowMenu->insertItem(i18n("&Close"), this, SLOT(closeActiveView()));
  int closeAllId = m_pWindowMenu->insertItem(i18n("Close &All"), this, SLOT(closeAllViews()));
  if (bNoViewOpened) {
    m_pWindowMenu->setItemEnabled(closeId, false);
    m_pWindowMenu->setItemEnabled(closeAllId, false);
  }
  if (!bTabPageMode && !bIDEAlMode) {
    int iconifyId = m_pWindowMenu->insertItem(i18n("&Minimize All"), this, SLOT(iconifyAllViews()));
    if (bNoViewOpened) {
      m_pWindowMenu->setItemEnabled(iconifyId, false);
    }
  }
  m_pWindowMenu->insertSeparator();
  m_pWindowMenu->insertItem(i18n("&MDI Mode"), m_pMdiModeMenu);
  m_pMdiModeMenu->clear();
  m_pMdiModeMenu->insertItem(i18n("&Toplevel Mode"), this, SLOT(switchToToplevelMode()));
  m_pMdiModeMenu->insertItem(i18n("C&hildframe Mode"), this, SLOT(switchToChildframeMode()));
  m_pMdiModeMenu->insertItem(i18n("Ta&b Page Mode"), this, SLOT(switchToTabPageMode()));
  m_pMdiModeMenu->insertItem(i18n("I&DEAl Mode"), this, SLOT(switchToIDEAlMode()));
  switch (m_mdiMode) {
    case KMdi::ToplevelMode:
      m_pMdiModeMenu->setItemChecked(m_pMdiModeMenu->idAt(0), true);
      break;
    case KMdi::ChildframeMode:
      m_pMdiModeMenu->setItemChecked(m_pMdiModeMenu->idAt(1), true);
      break;
    case KMdi::TabPageMode:
      m_pMdiModeMenu->setItemChecked(m_pMdiModeMenu->idAt(2), true);
      break;
    case KMdi::IDEAlMode:
      m_pMdiModeMenu->setItemChecked(m_pMdiModeMenu->idAt(3),true);
      break;
    default:
      break;
  }
  m_pWindowMenu->insertSeparator();
  if (!bTabPageMode && !bIDEAlMode) {
    int placMenuId = m_pWindowMenu->insertItem(i18n("&Tile"), m_pPlacingMenu);
    m_pPlacingMenu->clear();
    m_pPlacingMenu->insertItem(i18n("Ca&scade Windows"), m_pMdi,SLOT(cascadeWindows()));
    m_pPlacingMenu->insertItem(i18n("Cascade &Maximized"), m_pMdi,SLOT(cascadeMaximized()));
    m_pPlacingMenu->insertItem(i18n("Expand &Vertically"), m_pMdi,SLOT(expandVertical()));
    m_pPlacingMenu->insertItem(i18n("Expand &Horizontally"), m_pMdi,SLOT(expandHorizontal()));
    m_pPlacingMenu->insertItem(i18n("Tile &Non-overlapped"), m_pMdi,SLOT(tileAnodine()));
    m_pPlacingMenu->insertItem(i18n("Tile Overla&pped"), m_pMdi,SLOT(tilePragma()));
    m_pPlacingMenu->insertItem(i18n("Tile V&ertically"), m_pMdi,SLOT(tileVertically()));
    if (m_mdiMode == KMdi::ToplevelMode) {
      m_pWindowMenu->setItemEnabled(placMenuId, false);
    }
    m_pWindowMenu->insertSeparator();
    int dockUndockId = m_pWindowMenu->insertItem(i18n("&Dock/Undock"), m_pDockMenu);
    m_pDockMenu->clear();
    m_pWindowMenu->insertSeparator();
    if (bNoViewOpened) {
      m_pWindowMenu->setItemEnabled(placMenuId, false);
      m_pWindowMenu->setItemEnabled(dockUndockId, false);
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
    QDateTime timeStamp( pView->getTimeStamp() );

    if (pView->isToolView()) {
      continue;
    }

    QString item;
    // set titles of minimized windows in brackets
    if (pView->isMinimized()) {
      item += "(";
      item += pView->caption();
      item += ")";
    }
    else {
      item += " ";
      item += pView->caption();
    }

    // insert the window entry sorted by access time
    unsigned int indx;
    unsigned int windowItemCount = m_pWindowMenu->count() - entryCount;
    bool inserted = false;
    QString tmpString;
    QValueList<QDateTime>::iterator timeStampIterator = timeStamps.begin();
    for (indx = 0; indx <= windowItemCount; indx++, ++timeStampIterator) {
      bool putHere = false;
      if ((*timeStampIterator) < timeStamp) {
        putHere = true;
        timeStamps.insert(timeStampIterator, timeStamp);
      }
      if (putHere) {
        m_pWindowMenu->insertItem( item, pView, SLOT(slot_clickedInWindowMenu()), 0, -1, indx+entryCount);
        if (pView == m_pCurrentWindow) {
          m_pWindowMenu->setItemChecked( m_pWindowMenu->idAt( indx+entryCount), true);
        }
        pView->setWindowMenuID(i);
        if (!bTabPageMode) {
          m_pDockMenu->insertItem( item, pView, SLOT(slot_clickedInDockMenu()), 0, -1, indx);
          if (pView->isAttached()) {
            m_pDockMenu->setItemChecked( m_pDockMenu->idAt( indx), true);
          }
        }
        inserted = true;
        break;
        indx = windowItemCount+1;  // break the loop
      }
    }
    if (!inserted) {  // append it
      m_pWindowMenu->insertItem( item, pView, SLOT(slot_clickedInWindowMenu()), 0, -1, windowItemCount+entryCount);
      if (pView == m_pCurrentWindow) {
        m_pWindowMenu->setItemChecked( m_pWindowMenu->idAt(windowItemCount+entryCount), true);
      }
      pView->setWindowMenuID( i);
      if (!bTabPageMode) {
        m_pDockMenu->insertItem( item, pView, SLOT(slot_clickedInDockMenu()), 0, -1, windowItemCount);
        if (pView->isAttached()) {
          m_pDockMenu->setItemChecked( m_pDockMenu->idAt(windowItemCount), true);
        }
      }
    }
    i++;
  }
}

//================ windowMenuItemActivated ===============//

void KMdiMainFrm::windowMenuItemActivated(int id)
{
  if (id < 100) return;
  id -= 100;
  KMdiChildView *pView = m_pDocumentViews->at( id);
  if (!pView) return;
  if (pView->isMinimized()) pView->minimize();
  if (m_mdiMode != KMdi::TabPageMode) {
    KMdiChildFrm* pTopChild = m_pMdi->topChild();
    if (pTopChild) {
      if ((pView == pTopChild->m_pClient) && pView->isAttached()) {
        return;
      }
    }
  }
  activateView( pView);
}

//================ dockMenuItemActivated ===============//

void KMdiMainFrm::dockMenuItemActivated(int id)
{
  if( id < 100) return;
  id -= 100;
  KMdiChildView *pView = m_pDocumentViews->at( id);
  if( !pView) return;
  if( pView->isMinimized()) pView->minimize();
  if( pView->isAttached()) {
    detachWindow( pView, true);
  }
  else {   // is detached
    attachWindow( pView, true);
  }
}

//================ popupWindowMenu ===============//

void KMdiMainFrm::popupWindowMenu(QPoint p)
{
  if (!isFakingSDIApplication()) {
    m_pWindowMenu->popup( p);
  }
}

//================ dragEndTimeOut ===============//
void KMdiMainFrm::dragEndTimeOut()
{
  // send drag end to all concerned views.
  KMdiChildView* pView;
  for (m_pDocumentViews->first(); (pView = m_pDocumentViews->current()) != 0L; m_pDocumentViews->next()) {
    KMdiChildFrmDragEndEvent   dragEndEvent(0L);
    QApplication::sendEvent(pView, &dragEndEvent);
  }
}

//================ setFrameDecorOfAttachedViews ===============//

void KMdiMainFrm::setFrameDecorOfAttachedViews( int frameDecor)
{
  switch (frameDecor) {
    case 0:
      m_frameDecoration = KMdi::Win95Look;
      break;
    case 1:
      m_frameDecoration = KMdi::KDE1Look;
      break;
    case 2:
      m_frameDecoration = KMdi::KDELook;
      break;
    case 3:
      m_frameDecoration = KMdi::KDELaptopLook;
      break;
    default:
      qDebug("unknown MDI decoration");
      break;
  }
  setMenuForSDIModeSysButtons( m_pMainMenuBar);
  QPtrListIterator<KMdiChildView> it( *m_pDocumentViews);
  for( ; it.current(); ++it) {
    KMdiChildView* pView = it.current();
    if( pView->isToolView())
      continue;
    if( pView->isAttached())
      pView->mdiParent()->redecorateButtons();
  }
}

void KMdiMainFrm::fakeSDIApplication()
{
  m_bSDIApplication = true;
  if (m_pTaskBar)
    m_pTaskBar->close();
  m_pTaskBar = 0L;
}

void KMdiMainFrm::closeViewButtonPressed()
{
  KMdiChildView* pView = activeWindow();
  if (pView) {
    pView->close();
  }
}

void KMdiMainFrm::setManagedDockPositionModeEnabled(bool enabled)
{
  m_managedDockPositionMode=enabled;
}

void KMdiMainFrm::setActiveToolDock(KMdiDockContainer* td) {
  if (td==d->activeDockPriority[0]) return;
  if (d->activeDockPriority[0]==0) {
    d->activeDockPriority[0]=td;
    //        d->focusList=new KMdiFocusList(this);
    //        if (m_pMdi)  d->focusList->addWidgetTree(m_pMdi);
    //        if (m_documentTabWidget) d->focusList->addWidgetTree(m_documentTabWidget);
    return;
  }
  for (int dst=3,src=2;src>=0;dst--,src--) {
    if (d->activeDockPriority[src]==td) src--;
    if (src<0) break;
    d->activeDockPriority[dst]=d->activeDockPriority[src];
  }
  d->activeDockPriority[0]=td;
}

void KMdiMainFrm::removeFromActiveDockList(KMdiDockContainer* td) {
  for (int i=0;i<4;i++) {
    if (d->activeDockPriority[i]==td) {
      for (;i<3;i++)
        d->activeDockPriority[i]=d->activeDockPriority[i+1];
      d->activeDockPriority[3]=0;
      break;
    }
  }
  /*
        if (d->activeDockPriority[0]==0) {
        if (d->focusList) d->focusList->restore();
        delete d->focusList;
        d->focusList=0;
        }
   */
}

void KMdiMainFrm::prevToolViewInDock() {
  KMdiDockContainer* td=d->activeDockPriority[0];
  if (!td) return;
  td->prevToolView();
}

void KMdiMainFrm::nextToolViewInDock() {
  KMdiDockContainer* td=d->activeDockPriority[0];
  if (!td) return;
  td->nextToolView();
}

KMdi::TabWidgetVisibility KMdiMainFrm::tabWidgetVisibility()
{
  if ( m_documentTabWidget )
    return m_documentTabWidget->tabWidgetVisibility();

  return KMdi::NeverShowTabs;
}

void KMdiMainFrm::setTabWidgetVisibility( KMdi::TabWidgetVisibility visibility )
{
  if ( m_documentTabWidget )
    m_documentTabWidget->setTabWidgetVisibility( visibility );
}

KTabWidget * KMdiMainFrm::tabWidget() const
{
  return m_documentTabWidget;
}

#include "kmdimainfrm.moc"

// vim: ts=2 sw=2 et
// kate: space-indent on; indent-width 2; replace-tabs on;
