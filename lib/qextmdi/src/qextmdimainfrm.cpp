//----------------------------------------------------------------------------
//    filename             : qextmdimainfrm.cpp
//----------------------------------------------------------------------------
//    Project              : Qt MDI extension
//
//    begin                : 07/1999       by Szymon Stefanek as part of kvirc
//                                         (an IRC application)
//    changes              : 09/1999       by Falk Brettschneider to create an
//                           - 06/2000     stand-alone Qt extension set of
//                                         classes and a Qt-based library
//    patches              : 02/2000       by Massimo Morin (mmorin@schedsys.com)
//                           */2000        by Lars Beikirch (Lars.Beikirch@gmx.net)
//                           01/2003       by Jens Zurheide (jens.zurheide@gmx.de)
//
//    copyright            : (C) 1999-2000 by Szymon Stefanek (stefanek@tin.it)
//                                         and
//                                         Falk Brettschneider
//    email                :  gigafalk@yahoo.com (Falk Brettschneider)
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

#include <qcursor.h>
#include <qclipboard.h>
#include <qobjcoll.h>
#ifndef NO_KDE
#include <kmenubar.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kdeversion.h>
#include <qtabwidget.h>
#endif
#include <qtoolbutton.h>
#include <qlayout.h>
#include <qtimer.h>
#include <qtextstream.h>
#include <qstring.h>
#include <qmap.h>
#include <qvaluelist.h>

#include "qextmdimainfrm.h"
#include "qextmditaskbar.h"
#include "qextmdichildfrm.h"
#include "qextmdichildarea.h"
#include "qextmdichildview.h"

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

#ifndef NO_KDE
#include <X11/X.h>
#include <X11/Xlib.h>
#endif

#ifdef KeyRelease
/* I hate the defines in the X11 header files. Get rid of one of them */
#undef KeyRelease
#endif

#ifdef KeyPress
/* I hate the defines in the X11 header files. Get rid of one of them */
#undef KeyPress
#endif

using namespace KParts;

#if defined(_OS_WIN32_) || defined(Q_OS_WIN32)
QextMdi::FrameDecor QextMdiMainFrm::m_frameDecoration = QextMdi::Win95Look;
#else
QextMdi::FrameDecor QextMdiMainFrm::m_frameDecoration = QextMdi::KDELook;
#endif

QextMdi::MdiMode QextMdiMainFrm::m_mdiMode = QextMdi::ChildframeMode;

//============ constructor ============//
 QextMdiMainFrm::QextMdiMainFrm(QWidget* parentWidget, const char* name, WFlags flags)
: KParts::DockMainWindow( parentWidget, name, flags)
   ,m_pMdi(0L)
   ,m_pTaskBar(0L)
   ,m_pWinList(0L)
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
   ,m_bMaximizedChildFrmMode(FALSE)
   ,m_oldMainFrmHeight(0)
   ,m_oldMainFrmMinHeight(0)
   ,m_oldMainFrmMaxHeight(0)
   ,m_bSDIApplication(FALSE)
   ,m_pDockbaseAreaOfDocumentViews(0L)
   ,m_pDockbaseOfTabPage(0L)
   ,m_pTempDockSession(0L)
   ,m_bClearingOfWindowMenuBlocked(FALSE)
   ,m_pDragEndTimer(0L)
   ,m_bSwitching(FALSE)
{
   // Create the local list of windows
   m_pWinList = new QPtrList<QextMdiChildView>;
   m_pWinList->setAutoDelete(FALSE);
   // This seems to be needed (re-check it after Qt2.0 comed out)
   setFocusPolicy(ClickFocus);

   // create the central widget
   createMdiManager();

   // cover QextMdi's childarea by a dockwidget
   m_pDockbaseAreaOfDocumentViews = createDockWidget( "mdiAreaCover", QPixmap(), 0L, "mdi_area_cover");
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
   m_pWindowMenu->setCheckable( TRUE);
   QObject::connect( m_pWindowMenu, SIGNAL(aboutToShow()), this, SLOT(fillWindowMenu()) );

   m_pDockMenu = new QPopupMenu( this, "dock_menu");
   m_pDockMenu->setCheckable( TRUE);

   m_pMdiModeMenu = new QPopupMenu( this, "mdimode_menu");
   m_pMdiModeMenu->setCheckable( TRUE);

   m_pPlacingMenu = new QPopupMenu( this, "placing_menu");

   m_pDockbaseOfTabPage = m_pDockbaseAreaOfDocumentViews;

   // the MDI view taskbar
   createTaskBar();

   // drag end timer
   m_pDragEndTimer = new QTimer();
   connect(m_pDragEndTimer, SIGNAL(timeout()), this, SLOT(dragEndTimeOut()));
}

//============ ~QextMdiMainFrm ============//
QextMdiMainFrm::~QextMdiMainFrm()
{
   // safely close the windows so properties are saved...
   QextMdiChildView *pWnd = 0L;
   while((pWnd = m_pWinList->first()))closeWindow(pWnd, FALSE); // without re-layout taskbar!
   emit lastChildViewClosed();
   delete m_pWinList;
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
}

//============ applyOptions ============//
void QextMdiMainFrm::applyOptions()
{
   for(QextMdiChildView *w = m_pWinList->first();w;w= m_pWinList->next()){
      QWidget *wdgt = w;
      if(w->mdiParent())wdgt =w->mdiParent();
      // Really ugly hack to FORCE the resize event
      // a resize(width(),height()) won't work...
      wdgt->resize(wdgt->width()+1,wdgt->height()+1);
      wdgt->resize(wdgt->width()-1,wdgt->height()-1);
   }
}

//============ createMdiManager ============//
void QextMdiMainFrm::createMdiManager()
{
   m_pMdi=new QextMdiChildArea(this);
   setCentralWidget(m_pMdi);
   QObject::connect( m_pMdi, SIGNAL(nowMaximized(bool)), this, SLOT(setEnableMaximizedChildFrmMode(bool)) );
   QObject::connect( m_pMdi, SIGNAL(noMaximizedChildFrmLeft(QextMdiChildFrm*)), this, SLOT(switchOffMaximizeModeForMenu(QextMdiChildFrm*)) );
   QObject::connect( m_pMdi, SIGNAL(sysButtonConnectionsMustChange(QextMdiChildFrm*,QextMdiChildFrm*)), this, SLOT(updateSysButtonConnections(QextMdiChildFrm*,QextMdiChildFrm*)) );
   QObject::connect( m_pMdi, SIGNAL(popupWindowMenu(QPoint)), this, SLOT(popupWindowMenu(QPoint)) );
   QObject::connect( m_pMdi, SIGNAL(lastChildFrmClosed()), this, SIGNAL(lastChildFrmClosed()) );
#ifndef NO_KDE   
#if !(KDE_VERSION > 305)
   m_pMdi->installEventFilter( this );
#endif
#endif
}

//============ createTaskBar ==============//
void QextMdiMainFrm::createTaskBar()
{
   m_pTaskBar = new QextMdiTaskBar(this,QMainWindow::DockBottom);
   m_pTaskBar->installEventFilter( this);
}

void QextMdiMainFrm::slot_toggleTaskBar()
{
   if (!m_pTaskBar)
      return;
   m_pTaskBar->switchOn( !m_pTaskBar->isSwitchedOn());
}

void QextMdiMainFrm::resizeEvent(QResizeEvent *e)
{
   if( (m_mdiMode == QextMdi::ToplevelMode) && !parentWidget())
      if( e->oldSize().height() != e->size().height()) {
         return;
      }
   DockMainWindow::resizeEvent(e);
}

//================ setMinimumSize ===============//

void QextMdiMainFrm::setMinimumSize( int minw, int minh)
{
   if( (m_mdiMode == QextMdi::ToplevelMode) && !parentWidget())
      return;
   DockMainWindow::setMinimumSize( minw, minh);
}

//================ wrapper ===============//

QextMdiChildView* QextMdiMainFrm::createWrapper(QWidget *view, const QString& name, const QString& shortName)
{
  Q_ASSERT( view ); // if this assert fails, then some part didn't return a widget. Fix the part ;)

  QextMdiChildView* pMDICover = new QextMdiChildView(name);
  QBoxLayout* pLayout = new QHBoxLayout( pMDICover, 0, -1, "layout");
  view->reparent(pMDICover, QPoint(0,0));
  pLayout->addWidget(view);
  pMDICover->setName(name);
  pMDICover->setTabCaption(shortName);
  pMDICover->setCaption(name);

  const QPixmap* wndIcon = view->icon();
  if (wndIcon) {
      pMDICover->setIcon(*wndIcon);
  }

  return pMDICover;
}

//================ addWindow ===============//

void QextMdiMainFrm::addWindow( QextMdiChildView* pWnd, int flags)
{
   if( windowExists( pWnd)) {
      // is already added to the MDI system
      return;
   }

   if( flags & QextMdi::ToolWindow) {
      addToolWindow( pWnd);
      // some kind of cascading
      pWnd->move(m_pMdi->mapToGlobal(m_pMdi->getCascadePoint()));

      return;
   }

#ifndef NO_KDE
#if !(KDE_VERSION > 305)
   pWnd->installEventFilter( this );
#endif
#endif

   // common connections used when under MDI control
   QObject::connect( pWnd, SIGNAL(clickedInWindowMenu(int)), this, SLOT(windowMenuItemActivated(int)) );
   QObject::connect( pWnd, SIGNAL(focusInEventOccurs(QextMdiChildView*)), this, SLOT(activateView(QextMdiChildView*)) );
   QObject::connect( pWnd, SIGNAL(childWindowCloseRequest(QextMdiChildView*)), this, SLOT(childWindowCloseRequest(QextMdiChildView*)) );
   QObject::connect( pWnd, SIGNAL(attachWindow(QextMdiChildView*,bool)), this, SLOT(attachWindow(QextMdiChildView*,bool)) );
   QObject::connect( pWnd, SIGNAL(detachWindow(QextMdiChildView*,bool)), this, SLOT(detachWindow(QextMdiChildView*,bool)) );
   QObject::connect( pWnd, SIGNAL(clickedInDockMenu(int)), this, SLOT(dockMenuItemActivated(int)) );
   m_pWinList->append(pWnd);
   if (m_pTaskBar) {
      QextMdiTaskBarButton* but = m_pTaskBar->addWinButton(pWnd);
      QObject::connect( pWnd, SIGNAL(tabCaptionChanged(const QString&)), but, SLOT(setNewText(const QString&)) );
   }

   // embed the view depending on the current MDI mode
   if (m_mdiMode == QextMdi::TabPageMode) {
      const QPixmap& wndIcon = pWnd->icon() ? *(pWnd->icon()) : QPixmap();
      KDockWidget* pCover = createDockWidget( pWnd->name(),
                                              wndIcon,
                                              0L,  // parent
                                              pWnd->caption(),
                                              pWnd->tabCaption());
      pCover->setWidget( pWnd);
      pCover->setToolTipString( pWnd->caption());
      if (!(flags & QextMdi::Detach)) {
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
         m_pDockbaseOfTabPage = pCover;
      }
      if (!(flags & QextMdi::Hide)) {
         pCover->show();
      }
      pWnd->setFocus();
      if (m_pWinList->count() == 1) {
         m_pClose->show();  // show the close button in case it isn't already
      }
   }
   else {
      if( (flags & QextMdi::Detach) || (m_mdiMode == QextMdi::ToplevelMode)) {
         detachWindow( pWnd, !(flags & QextMdi::Hide));
         emit childViewIsDetachedNow(pWnd); // fake it because detach won't call it in this case of addWindow-to-MDI
      } else {
         attachWindow( pWnd, !(flags & QextMdi::Hide), flags & QextMdi::UseQextMDISizeHint);
      }

      if ((m_bMaximizedChildFrmMode && !(m_bSDIApplication && (flags & QextMdi::Detach)) && (m_mdiMode != QextMdi::ToplevelMode))
       || (flags & QextMdi::Maximize) ) {
         if (!pWnd->isMaximized())
            pWnd->maximize();
      }
      if (!m_bSDIApplication || (flags & QextMdi::Detach)) {
         if (flags & QextMdi::Minimize)
         {
            pWnd->minimize();
          }
         if (!(flags & QextMdi::Hide)) {
            if (pWnd->isAttached()) {
               pWnd->mdiParent()->show();
            }
            else {
               pWnd->show();
            }
         }
      }
   }
}

//============ addWindow ============//
void QextMdiMainFrm::addWindow( QextMdiChildView* pWnd, QRect rectNormal, int flags)
{
   addWindow( pWnd, flags);
   if (m_bMaximizedChildFrmMode && pWnd->isAttached()) {
      pWnd->setRestoreGeometry( rectNormal);
   } else {
      pWnd->setGeometry( rectNormal);
   }
}

//============ addWindow ============//
void QextMdiMainFrm::addWindow( QextMdiChildView* pWnd, QPoint pos, int flags)
{
   addWindow( pWnd, flags);
   if (m_bMaximizedChildFrmMode && pWnd->isAttached()) {
      pWnd->setRestoreGeometry( QRect(pos, pWnd->restoreGeometry().size()));
   } else {
      pWnd->move( pos);
   }
}

//============ addWindow ============//
void QextMdiMainFrm::addToolWindow( QWidget* pWnd, KDockWidget::DockPosition pos, QWidget* pTargetWnd, int percent, const QString& tabToolTip, const QString& tabCaption)
{
   QextMdiChildView* pToolView = 0L;
   QRect r = pWnd->geometry();

   // if pWnd is not a QextMdiChildView already, cover it by such widget
   if (pWnd->inherits("QextMdiChildView")) {
      pToolView = (QextMdiChildView*) pWnd;
   }
   else {
      pToolView = new QextMdiChildView( pWnd->caption());
      QHBoxLayout* pLayout = new QHBoxLayout( pToolView, 0, -1, "internal_qextmdichildview_layout");
      pWnd->reparent( pToolView, QPoint(0,0));
      pToolView->setName( pWnd->name());
      pToolView->setFocusPolicy( pWnd->focusPolicy());
      const QPixmap& wndIcon = pWnd->icon() ? *(pWnd->icon()) : QPixmap();
      pToolView->setIcon( wndIcon);
      pToolView->setCaption( pWnd->caption());
      QApplication::sendPostedEvents();
      pLayout->addWidget( pWnd);
   }

   // if docking is not desired, add the toolview as stay-on-top toplevel view
   if (pos == KDockWidget::DockNone) {
      pToolView->reparent(this, Qt::WType_TopLevel | Qt::WType_Dialog, r.topLeft(), pToolView->isVisible());
      QObject::connect( pToolView, SIGNAL(childWindowCloseRequest(QextMdiChildView*)), this, SLOT(childWindowCloseRequest(QextMdiChildView*)) );
      QObject::connect( pToolView, SIGNAL(focusInEventOccurs(QextMdiChildView*)), this, SLOT(activateView(QextMdiChildView*)) );
      m_pWinList->append(pToolView);
      pToolView->m_bToolView = TRUE;
      pToolView->setGeometry(r);
   }
   else {   // add (and dock) the toolview as DockWidget view
      const QPixmap& wndIcon = pWnd->icon() ? *(pWnd->icon()) : QPixmap();
      KDockWidget* pCover = createDockWidget( pToolView->name(),
                                              wndIcon,
                                              0L,  // parent
                                              pToolView->caption(),
                                              tabCaption );
      pCover->setWidget( pToolView);
      pCover->setToolTipString( tabToolTip);
      KDockWidget* pTargetDock = 0L;
        // Should we dock to ourself?
      bool DockToOurself = FALSE;
      if(m_pDockbaseAreaOfDocumentViews)
      {
        if (pTargetWnd == m_pDockbaseAreaOfDocumentViews->getWidget()) DockToOurself = TRUE;
      }
      if (pTargetWnd == this) DockToOurself = TRUE;
      if (DockToOurself) pTargetDock = m_pDockbaseAreaOfDocumentViews;
      else if(pTargetWnd != 0L) {
         pTargetDock = dockManager->findWidgetParentDock( pTargetWnd);
         if (!pTargetDock) {
            if (pTargetWnd->parentWidget() && pTargetWnd->parentWidget()->inherits("QextMdiChildView")) {
               pTargetDock = dockManager->findWidgetParentDock( pTargetWnd->parentWidget());
            }
         }
      }
      if (pTargetDock)
         pCover->manualDock( pTargetDock, pos, percent);
//check      pCover->show();
   }
}

//============ attachWindow ============//
void QextMdiMainFrm::attachWindow(QextMdiChildView *pWnd, bool bShow, bool bAutomaticResize)
{
   pWnd->installEventFilter(this);

   // decide whether window shall be cascaded
   bool bCascade = FALSE;
   QApplication::sendPostedEvents();
   QRect frameGeo = pWnd->frameGeometry();
   QPoint topLeftScreen = pWnd->mapToGlobal(QPoint(0,0));
   QPoint topLeftMdiChildArea = m_pMdi->mapFromGlobal(topLeftScreen);
   QRect childAreaGeo = m_pMdi->geometry();
   if ( (topLeftMdiChildArea.x() < 0) || (topLeftMdiChildArea.y() < 0) ||
        (topLeftMdiChildArea.x()+frameGeo.width() > childAreaGeo.width()) ||
        (topLeftMdiChildArea.y()+frameGeo.height() > childAreaGeo.height()) ) {
      bCascade = TRUE;
   }

   // create frame and insert child view
   QextMdiChildFrm *lpC=new QextMdiChildFrm(m_pMdi);
   pWnd->hide();
   if (!bCascade) {
      lpC->move(topLeftMdiChildArea);
   }
   lpC->setClient(pWnd, bAutomaticResize);
   lpC->setFocus();
   pWnd->youAreAttached(lpC);
   if( (m_mdiMode == QextMdi::ToplevelMode) && !parentWidget()) {
      setMinimumHeight( m_oldMainFrmMinHeight);
      setMaximumHeight( m_oldMainFrmMaxHeight);
      resize( width(), m_oldMainFrmHeight);
      m_oldMainFrmHeight = 0;
      switchToChildframeMode();
   }

   m_pMdi->manageChild(lpC,FALSE,bCascade);
   if (m_pMdi->topChild() && m_pMdi->topChild()->isMaximized()) {
      QRect r = lpC->geometry();
      lpC->setGeometry(-lpC->m_pClient->x(), -lpC->m_pClient->y(),
                       m_pMdi->width()  + QEXTMDI_MDI_CHILDFRM_DOUBLE_BORDER,
                       m_pMdi->height() + lpC->captionHeight() + QEXTMDI_MDI_CHILDFRM_SEPARATOR + QEXTMDI_MDI_CHILDFRM_DOUBLE_BORDER);
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
void QextMdiMainFrm::detachWindow(QextMdiChildView *pWnd, bool bShow)
{
   if (pWnd->isAttached()) {
      pWnd->removeEventFilter(this);
      pWnd->youAreDetached();
      // this is only if it was attached and you want to detach it
      if (pWnd->parent()) {
         QextMdiChildFrm *lpC=pWnd->mdiParent();
         if (lpC) {
            if (lpC->icon()) {
               QPixmap pixm(*(lpC->icon()));
               pWnd->setIcon(pixm);
            }
            QString capt(lpC->caption());
            if (!bShow)
               lpC->hide();
            lpC->unsetClient( m_undockPositioningOffset);
            m_pMdi->destroyChildButNotItsView(lpC,FALSE); //Do not focus the new top child , we loose focus...
            pWnd->setCaption(capt);
         }
      }
   }
   else {
      if (pWnd->size().isEmpty() || (pWnd->size() == QSize(1,1))) {
         if (m_pCurrentWindow) {
            pWnd->setGeometry( QRect( m_pMdi->getCascadePoint(m_pWinList->count()-1), m_pCurrentWindow->size()));
         }
         else {
            pWnd->setGeometry( QRect( m_pMdi->getCascadePoint(m_pWinList->count()-1), defaultChildFrmSize()));
         }
      }
#ifndef NO_KDE
      if (mdiMode() == QextMdi::ToplevelMode) {
         XSetTransientForHint(qt_xdisplay(),pWnd->winId(),topLevelWidget()->winId());
      }
#endif

      return;
   }
#ifndef NO_KDE
   if (mdiMode() == QextMdi::ToplevelMode) {
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
void QextMdiMainFrm::removeWindowFromMdi(QextMdiChildView *pWnd)
{
   //Closes a child window. sends no close event : simply deletes it
   if (!(m_pWinList->removeRef(pWnd)))
      return;
   if (m_pWinList->count() == 0)
     m_pCurrentWindow = 0L;

   QObject::disconnect( pWnd, SIGNAL(attachWindow(QextMdiChildView*,bool)), this, SLOT(attachWindow(QextMdiChildView*,bool)) );
   QObject::disconnect( pWnd, SIGNAL(detachWindow(QextMdiChildView*,bool)), this, SLOT(detachWindow(QextMdiChildView*,bool)) );
   QObject::disconnect( pWnd, SIGNAL(focusInEventOccurs(QextMdiChildView*)), this, SLOT(activateView(QextMdiChildView*)) );
   QObject::disconnect( pWnd, SIGNAL(childWindowCloseRequest(QextMdiChildView*)), this, SLOT(childWindowCloseRequest(QextMdiChildView*)) );
   QObject::disconnect( pWnd, SIGNAL(clickedInWindowMenu(int)), this, SLOT(windowMenuItemActivated(int)) );
   QObject::disconnect( pWnd, SIGNAL(clickedInDockMenu(int)), this, SLOT(dockMenuItemActivated(int)) );

   if (m_pTaskBar) {
      QextMdiTaskBarButton* but = m_pTaskBar->getButton(pWnd);
      if (but != 0L) {
         QObject::disconnect( pWnd, SIGNAL(tabCaptionChanged(const QString&)), but, SLOT(setNewText(const QString&)) );
      }
      m_pTaskBar->removeWinButton(pWnd);
   }

   if (m_mdiMode == QextMdi::TabPageMode) {
      if (m_pWinList->count() == 0) {
         if (!m_pDockbaseAreaOfDocumentViews) {
            m_pDockbaseAreaOfDocumentViews = createDockWidget( "mdiAreaCover", QPixmap(), 0L, "mdi_area_cover");
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
         QextMdiChildView* pView = m_pMdi->topChild()->m_pClient;
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
      pWnd->m_bToolView = FALSE;

   if (!m_pCurrentWindow)
      emit lastChildViewClosed();
}

//============== closeWindow ==============//
void QextMdiMainFrm::closeWindow(QextMdiChildView *pWnd, bool layoutTaskBar)
{
   //Closes a child window. sends no close event : simply deletes it
   m_pWinList->removeRef(pWnd);
   if (m_pWinList->count() == 0)
      m_pCurrentWindow = 0L;

   if (m_pTaskBar) {
      m_pTaskBar->removeWinButton(pWnd, layoutTaskBar);
   }

   if (m_mdiMode == QextMdi::TabPageMode) {
      if (m_pWinList->count() == 0) {
         if (!m_pDockbaseAreaOfDocumentViews) {
            m_pDockbaseAreaOfDocumentViews = createDockWidget( "mdiAreaCover", QPixmap(), 0L, "mdi_area_cover");
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
      m_pMdi->destroyChild(pWnd->mdiParent());
   }
   else {
      delete pWnd;
      // is not attached
      if (m_pMdi->getVisibleChildCount() > 0) {
         setActiveWindow();
         m_pCurrentWindow = 0L;
         QextMdiChildView* pView = m_pMdi->topChild()->m_pClient;
         if (pView) {
            pView->activate();
         }
      }
      else if (m_pWinList->count() > 0) {
         m_pWinList->last()->activate();
         m_pWinList->last()->setFocus();
      }
   }

   if (!m_pCurrentWindow)
      emit lastChildViewClosed();
}

//================== findWindow =================//
QextMdiChildView * QextMdiMainFrm::findWindow(const QString& caption)
{
   for(QextMdiChildView *w=m_pWinList->first();w;w=m_pWinList->next()){
      if(QString(w->caption()) == QString(caption))return w;   //F.B.
   }
   return 0L;
}

//================== activeWindow ===================//
QextMdiChildView * QextMdiMainFrm::activeWindow()
{
   return m_pCurrentWindow;
}

//================== windowExists ? =================//
bool QextMdiMainFrm::windowExists(QextMdiChildView *pWnd)
{
   for(QextMdiChildView *w=m_pWinList->first();w;w=m_pWinList->next()){
      if(w == pWnd)return TRUE;
   }
   return FALSE;
}

QPopupMenu * QextMdiMainFrm::windowPopup(QextMdiChildView * pWnd,bool bIncludeTaskbarPopup)
{
   m_pWindowPopup->clear();
   if(bIncludeTaskbarPopup){
      m_pWindowPopup->insertItem(tr("Window"),taskBarPopup(pWnd,FALSE));
      m_pWindowPopup->insertSeparator();
   }
   return m_pWindowPopup;
}

//================ taskBarPopup =================//
QPopupMenu * QextMdiMainFrm::taskBarPopup(QextMdiChildView *pWnd,bool /*bIncludeWindowPopup*/)
{
   //returns the g_pTaskBarPopup filled according to the QextMdiChildView state
   m_pTaskBarPopup->clear();
   if(pWnd->isAttached()){
      m_pTaskBarPopup->insertItem(tr("Undock"),pWnd,SLOT(detach()));
      m_pTaskBarPopup->insertSeparator();
      if(pWnd->isMinimized() || pWnd->isMaximized())
         m_pTaskBarPopup->insertItem(tr("Restore"),pWnd,SLOT(restore()));
      if(!pWnd->isMaximized())m_pTaskBarPopup->insertItem(tr("Maximize"),pWnd,SLOT(maximize()));
      if(!pWnd->isMinimized())m_pTaskBarPopup->insertItem(tr("Minimize"),pWnd,SLOT(minimize()));
   } else m_pTaskBarPopup->insertItem(tr("Dock"),pWnd,SLOT(attach()));
   m_pTaskBarPopup->insertSeparator();
   m_pTaskBarPopup->insertItem(tr("Close"),pWnd,SLOT(close()));
   // the window has a view...get the window popup
   m_pTaskBarPopup->insertSeparator();
   m_pTaskBarPopup->insertItem(tr("Operations"),windowPopup(pWnd,FALSE));  //alvoid recursion
   return m_pTaskBarPopup;
}

void QextMdiMainFrm::activateView(QextMdiChildView* pWnd)
{
   pWnd->m_bMainframesActivateViewIsPending = TRUE;

	 bool bActivateNecessary = TRUE;
   if (m_pCurrentWindow != pWnd) {
      m_pCurrentWindow = pWnd;
   }
   else {
      bActivateNecessary = FALSE;
			// if this method is called as answer to view->activate(),
			// interrupt it because it's not necessary
      pWnd->m_bInterruptActivation = TRUE;
   }

   if (m_pTaskBar) {
      m_pTaskBar->setActiveButton(pWnd);
   }

   if (m_mdiMode == QextMdi::TabPageMode) {
      makeWidgetDockVisible(pWnd);
      m_pDockbaseOfTabPage = (KDockWidget*) pWnd->parentWidget();
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
//         if (!pWnd->hasFocus()) {
//            pWnd->setFocus();
//         }
      }
   }
   pWnd->m_bMainframesActivateViewIsPending = FALSE;
}

void QextMdiMainFrm::taskbarButtonRightClicked(QextMdiChildView *pWnd)
{
   activateView( pWnd); // set focus
   QApplication::sendPostedEvents();
   taskBarPopup( pWnd, TRUE)->popup( QCursor::pos());
}

void QextMdiMainFrm::childWindowCloseRequest(QextMdiChildView *pWnd)
{
   QextMdiViewCloseEvent* ce = new QextMdiViewCloseEvent( pWnd);
   QApplication::postEvent( this, ce);
}

bool QextMdiMainFrm::event( QEvent* e)
{
   if( e->type() == QEvent::User) {
      QextMdiChildView* pWnd = (QextMdiChildView*)((QextMdiViewCloseEvent*)e)->data();
      if( pWnd != 0L)
         closeWindow( pWnd);
      return TRUE;
   }
   // A hack little hack: If MDI child views are moved implicietly by moving 
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
   else if( isVisible() && (e->type() == QEvent::Move)) {
      if (m_pDragEndTimer->isActive()) {
         // this is not the first move -> stop old timer
         m_pDragEndTimer->stop();
      }
      else {
         // this is the first move -> send the drag begin to all concerned views 
         QextMdiChildView* pView;
         for (m_pWinList->first(); (pView = m_pWinList->current()) != 0L; m_pWinList->next()) {
            QextMdiChildFrmDragBeginEvent    dragBeginEvent(0L);
            QApplication::sendEvent(pView, &dragBeginEvent);
         }
      }
      m_pDragEndTimer->start(200, TRUE); // single shot after 200 ms
   }

   return DockMainWindow::event( e);
}

bool QextMdiMainFrm::eventFilter(QObject * /*obj*/, QEvent *e )
{
   if( e->type() == QEvent::FocusIn) {
      QFocusEvent* pFE = (QFocusEvent*) e;
      if (pFE->reason() == QFocusEvent::ActiveWindow) {
         if (m_pCurrentWindow && !m_pCurrentWindow->isHidden() && !m_pCurrentWindow->isAttached() && m_pMdi->topChild()) {
            return TRUE;   // eat the event
         }
      }
      if (m_pMdi) {
         static bool bFocusTCIsPending = FALSE;
         if (!bFocusTCIsPending) {
            bFocusTCIsPending = TRUE;
            m_pMdi->focusTopChild();
            bFocusTCIsPending = FALSE;
         }
      }
   }
   else if (e->type() == QEvent::KeyRelease) {
      if (switching()) {
#ifndef NO_KDE        
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
              if (state != ((QKeyEvent *)e)->stateAfter()                              &&
                   ((modFlags & KKey::CTRL) > 0) == ((state & Qt::ControlButton) > 0 ) &&
                   ((modFlags & KKey::ALT) > 0)  == ((state & Qt::AltButton) > 0)      &&
                   ((modFlags & KKey::WIN) > 0)  == ((state & Qt::MetaButton) > 0) )
              {
                activeWindow()->updateTimeStamp();
                setSwitching(FALSE);
              }
              return TRUE;
#else
    // TODO implement me for Qt-only!
#endif
        }
        else {
#ifdef NO_KDE
          qDebug( "### KAction( \"view_next_window\") __not__ found.\n" );
#else
          kdDebug(9000) << "KAction( \"view_next_window\") not found." << endl;
#endif          
        }
      }    
   }
   return FALSE;  // standard event processing
}

/**
 * close all views
 */
void QextMdiMainFrm::closeAllViews()
{
   for(QextMdiChildView *w = m_pWinList->first();w;w= m_pWinList->next()){
      w->close();
   }
}

/**
 * iconify all views
 */
void QextMdiMainFrm::iconifyAllViews()
{
   for(QextMdiChildView *w = m_pWinList->first();w;w= m_pWinList->next()){
      if(!w->isToolView())
         w->minimize();
   }
}

/**
 * closes the view of the active (topchild) window
 */
void QextMdiMainFrm::closeActiveView()
{
   if( m_pCurrentWindow != 0L) {
      m_pCurrentWindow->close();
   }
}

/** find the root dockwidgets and store their geometry */
void QextMdiMainFrm::findRootDockWidgets(QPtrList<KDockWidget>* pRootDockWidgetList, QValueList<QRect>* pPositionList)
{
   if (!pRootDockWidgetList) return;
   if (!pPositionList) return;

   // since we set some windows to toplevel, we must consider the window manager's window frame
   const int frameBorderWidth  = 7;  // TODO: Can we / do we need to ask the window manager?
   const int windowTitleHeight = 10; // TODO:    -"-

   QObjectList* pObjList = queryList( "KDockWidget");
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
         if (pW->inherits("KDockWidget")) {
            pUndockCandidate = (KDockWidget*) pW;
            if (pUndockCandidate->enableDocking() != KDockWidget::DockNone)
               pRootDockW = pUndockCandidate;
         }
         pW = pW->parentWidget();
      }
      if (pRootDockW) {
         // if that oldest ancestor is not already in the list, append it
         bool found = FALSE;
         QPtrListIterator<KDockWidget> it2( *pRootDockWidgetList);
         if (!pRootDockWidgetList->isEmpty()) {
            for ( ; it2.current() && !found; ++it2 ) {
               KDockWidget* pDockW = it2.current();
               if (pDockW == pRootDockW)
                  found = TRUE;
            }
            if (!found) {
               pRootDockWidgetList->append( pDockW);
               QPoint p = pDockW->mapToGlobal( pDockW->pos())-pDockW->pos();
               QRect r( p.x(),
                        p.y()+m_undockPositioningOffset.y(),
                        pDockW->width()  - windowTitleHeight - frameBorderWidth*2,
                        pDockW->height() - windowTitleHeight - frameBorderWidth*2);
               pPositionList->append( r);
            }
         }
         else {
            pRootDockWidgetList->append( pRootDockW);
            QPoint p = pRootDockW->mapToGlobal( pRootDockW->pos())-pRootDockW->pos();
            QRect r( p.x(),
                     p.y()+m_undockPositioningOffset.y(),
                     pRootDockW->width()  - windowTitleHeight - frameBorderWidth*2,
                     pRootDockW->height() - windowTitleHeight - frameBorderWidth*2);
            pPositionList->append( r);
         }
      }
   }
}

/**
 * undocks all view windows (unix-like)
 */
void QextMdiMainFrm::switchToToplevelMode()
{
   if (m_mdiMode == QextMdi::ToplevelMode)
      return;

   QextMdi::MdiMode oldMdiMode = m_mdiMode;

   const int frameBorderWidth  = 7;  // TODO: Can we / do we need to ask the window manager?
   setUndockPositioningOffset( QPoint( 0, (m_pTaskBar ? m_pTaskBar->height() : 0) + frameBorderWidth));

   // 1.) select the dockwidgets to be undocked and store their geometry
   QPtrList<KDockWidget> rootDockWidgetList;
   QValueList<QRect> positionList;
   findRootDockWidgets(&rootDockWidgetList, &positionList);

   // 2.) undock the MDI views of QextMDI
   if (oldMdiMode == QextMdi::ChildframeMode) {
      finishChildframeMode();
   }
   else if (oldMdiMode == QextMdi::TabPageMode) { // if tabified, release all views from their docking covers
      finishTabPageMode();
   }

   // 3.) undock all these found oldest ancestors (being KDockWidgets)
   QPtrListIterator<KDockWidget> it3( rootDockWidgetList);
   for (; it3.current(); ++it3 ) {
       KDockWidget* pDockW = it3.current();
       pDockW->undock();
   }

   // 4.) recreate the MDI childframe area and hide it
   if (oldMdiMode == QextMdi::TabPageMode) {
      if (!m_pDockbaseAreaOfDocumentViews) {
         m_pDockbaseAreaOfDocumentViews = createDockWidget( "mdiAreaCover", QPixmap(), 0L, "mdi_area_cover");
         m_pDockbaseAreaOfDocumentViews->setEnableDocking(KDockWidget::DockNone);
         m_pDockbaseAreaOfDocumentViews->setDockSite(KDockWidget::DockCorner);
         m_pDockbaseAreaOfDocumentViews->setWidget(m_pMdi);
      }
      // set this dock to main view
      setView(m_pDockbaseAreaOfDocumentViews);
      setMainDockWidget(m_pDockbaseAreaOfDocumentViews);
      m_pDockbaseOfTabPage = m_pDockbaseAreaOfDocumentViews;
   }
   QApplication::sendPostedEvents();
   if (!parentWidget()) {
      m_oldMainFrmMinHeight = minimumHeight();
      m_oldMainFrmMaxHeight = maximumHeight();
      m_oldMainFrmHeight = height();
      if( m_pWinList->count())
         setFixedHeight( height() - m_pDockbaseAreaOfDocumentViews->height());
      else { // consider space for the taskbar
         QApplication::sendPostedEvents();
         setFixedHeight( height() - m_pDockbaseAreaOfDocumentViews->height() + 27);
      }
   }

   // 5. show the child views again
   QPtrListIterator<QextMdiChildView> it( *m_pWinList);
   for( it.toFirst(); it.current(); ++it) {
      QextMdiChildView* pView = it.current();
#ifndef NO_KDE
      XSetTransientForHint(qt_xdisplay(),pView->winId(),winId());
#endif
      if( !pView->isToolView())
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
   m_mdiMode = QextMdi::ToplevelMode;
   //qDebug("ToplevelMode on");
}

void QextMdiMainFrm::finishToplevelMode()
{
   m_pDockbaseAreaOfDocumentViews->setDockSite(KDockWidget::DockCorner);
}

/**
 * docks all view windows (Windows-like)
 */
void QextMdiMainFrm::switchToChildframeMode()
{
   if (m_mdiMode == QextMdi::ChildframeMode)
      return;

   QPtrList<KDockWidget> rootDockWidgetList;
   if (m_mdiMode == QextMdi::TabPageMode) {
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
   }
   else if (m_mdiMode == QextMdi::ToplevelMode) {
      finishToplevelMode();
   }

   if (!m_pDockbaseAreaOfDocumentViews) {
      // cover QextMdi's childarea by a dockwidget
      m_pDockbaseAreaOfDocumentViews = createDockWidget( "mdiAreaCover", QPixmap(), 0L, "mdi_area_cover");
      m_pDockbaseAreaOfDocumentViews->setEnableDocking(KDockWidget::DockNone);
      m_pDockbaseAreaOfDocumentViews->setDockSite(KDockWidget::DockCorner);
      m_pDockbaseAreaOfDocumentViews->setWidget(m_pMdi);
   }
   if (m_pDockbaseAreaOfDocumentViews->isTopLevel()) {
      // set this dock to main view
      setView(m_pDockbaseAreaOfDocumentViews);
      setMainDockWidget(m_pDockbaseAreaOfDocumentViews);
      m_pDockbaseAreaOfDocumentViews->setEnableDocking(KDockWidget::DockNone);
      m_pDockbaseAreaOfDocumentViews->setDockSite(KDockWidget::DockCorner);
      m_pDockbaseOfTabPage = m_pDockbaseAreaOfDocumentViews;
   }
   m_pDockbaseAreaOfDocumentViews->show();
   if (m_mdiMode == QextMdi::TabPageMode) {
      QPtrListIterator<KDockWidget> it4( rootDockWidgetList);
      for (; it4.current(); ++it4 ) {
         KDockWidget* pDockW = it4.current();
         pDockW->dockBack();
      }
   }

   if (m_mdiMode == QextMdi::ToplevelMode && m_pTempDockSession) {
     // restore the old dock szenario which we memorized at the time we switched to toplevel mode
     QDomElement oldDockState = m_pTempDockSession->namedItem("cur_dock_state").toElement();
     readDockConfig( oldDockState);
   }

   QextMdi::MdiMode oldMdiMode = m_mdiMode;
   m_mdiMode = QextMdi::ChildframeMode;

   QPtrListIterator<QextMdiChildView> it( *m_pWinList);
   for( ; it.current(); ++it) {
      QextMdiChildView* pView = it.current();
      if( !pView->isToolView())
         if( !pView->isAttached())
            attachWindow( pView, TRUE);
   }
   for( it.toFirst(); it.current(); ++it) {
      QextMdiChildView* pView = it.current();
      if( !pView->isToolView())
         pView->show();
   }
   if( (oldMdiMode == QextMdi::ToplevelMode) && !parentWidget()) {
      setMinimumHeight( m_oldMainFrmMinHeight);
      setMaximumHeight( m_oldMainFrmMaxHeight);
      resize( width(), m_oldMainFrmHeight);
      m_oldMainFrmHeight = 0;
      //qDebug("TopLevelMode off");
      emit leftTopLevelMode();
   }
}

void QextMdiMainFrm::finishChildframeMode()
{
   // save the old dock szenario of the dockwidged-like tool views to a DOM tree
   delete m_pTempDockSession;
   m_pTempDockSession = new QDomDocument( "docksession");
   QDomElement curDockState = m_pTempDockSession->createElement("cur_dock_state");
   m_pTempDockSession->appendChild( curDockState);
   writeDockConfig( curDockState);

   // detach all non-tool-views to toplevel
   QPtrListIterator<QextMdiChildView> it( *m_pWinList);
   for( ; it.current(); ++it) {
      QextMdiChildView* pView = it.current();
      if( pView->isToolView())
         continue;
      if( pView->isAttached()) {
         if( pView->isMaximized())
            pView->mdiParent()->setGeometry( 0, 0, m_pMdi->width(), m_pMdi->height());
         detachWindow( pView, FALSE);
	 
      }
   }
}

/**
 * Docks all view windows (Windows-like)
 */
void QextMdiMainFrm::switchToTabPageMode()
{
   QextMdiChildView* pRemActiveWindow = activeWindow();

   if (m_mdiMode == QextMdi::TabPageMode)
      return;  // nothing need to be done

   // make sure that all MDI views are detached
   if (m_mdiMode == QextMdi::ChildframeMode) {
      finishChildframeMode();
   }
   else if (m_mdiMode == QextMdi::ToplevelMode) {
      finishToplevelMode();
   }

   // resize to childframe mode size of the mainwindow if we were in toplevel mode
   if( (m_mdiMode == QextMdi::ToplevelMode) && !parentWidget()) {
      setMinimumHeight( m_oldMainFrmMinHeight);
      setMaximumHeight( m_oldMainFrmMaxHeight);
      resize( width(), m_oldMainFrmHeight);
      m_oldMainFrmHeight = 0;
      //qDebug("TopLevelMode off");
      emit leftTopLevelMode();
      QApplication::sendPostedEvents();

      // restore the old dock szenario which we memorized at the time we switched to toplevel mode
      QDomElement oldDockState = m_pTempDockSession->namedItem("cur_dock_state").toElement();
      readDockConfig( oldDockState);
   }

   if (m_pDockbaseOfTabPage != m_pDockbaseAreaOfDocumentViews) {
      delete m_pDockbaseOfTabPage;
      m_pDockbaseOfTabPage = m_pDockbaseAreaOfDocumentViews;
   }

   m_mdiMode = QextMdi::TabPageMode;

   // tabify all MDI views covered by a KDockWidget
   KDockWidget* pCover = 0L;
   QPtrListIterator<QextMdiChildView> it4( *m_pWinList);
   for( ; it4.current(); ++it4) {
      QextMdiChildView* pView = it4.current();
      if( pView->isToolView())
         continue;
      const QPixmap& wndIcon = pView->icon() ? *(pView->icon()) : QPixmap();
      pCover = createDockWidget( pView->name(),
                                 wndIcon,
                                 0L,  // parent
                                 pView->caption(),
                                 pView->tabCaption());
      pCover->setWidget( pView);
      pCover->setToolTipString( pView->caption());
      m_pDockbaseOfTabPage->setDockSite(KDockWidget::DockFullSite);
      // dock as tab-page
      pCover->manualDock( m_pDockbaseOfTabPage, KDockWidget::DockCenter);
      // update the dockbase
      pCover->setEnableDocking(KDockWidget::DockNone);
      if (m_pDockbaseOfTabPage == m_pDockbaseAreaOfDocumentViews) {
         m_pMdi->reparent(0,QPoint(0,0));
         m_pDockbaseAreaOfDocumentViews->close();
         delete m_pDockbaseAreaOfDocumentViews;
         m_pDockbaseAreaOfDocumentViews = 0L;
         QApplication::sendPostedEvents();
      }
      else {
         m_pDockbaseOfTabPage->setDockSite(KDockWidget::DockCorner);
      }
      m_pDockbaseOfTabPage = pCover;
      setMainDockWidget(pCover);
   }
   if (pCover) {
      if (m_pWinList->count() > 1) { // note: with only 1 page we haven't already tabbed widgets
         // set the first page as active page
         QTabWidget* pTab = (QTabWidget*) pCover->parentWidget()->parentWidget();
         if (pTab)
            pTab->showPage(pRemActiveWindow);
      }
      pRemActiveWindow->setFocus();
   }

   m_pTaskBar->switchOn(FALSE);

   QObject::connect( m_pClose, SIGNAL(clicked()), this, SLOT(closeViewButtonPressed()) );
   if (m_pWinList->count() > 0) {
      m_pClose->show();
   }
   //qDebug("TabPageMode on");
}

void QextMdiMainFrm::finishTabPageMode()
{
   // if tabified, release all views from their docking covers
   if (m_mdiMode == QextMdi::TabPageMode) {
      m_pClose->hide();
      QObject::disconnect( m_pClose, SIGNAL(clicked()), this, SLOT(closeViewButtonPressed()) );
      
      QPtrListIterator<QextMdiChildView> it( *m_pWinList);
      for( ; it.current(); ++it) {
         QextMdiChildView* pView = it.current();
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
         ((KDockWidget*)pParent)->undock(); // this destroys the dockwiget cover, too
         pParent->close();
         delete pParent;
         if (centralWidget() == pParent) {
            setCentralWidget(0L); // avoid dangling pointer
         }
      }
      m_pTaskBar->switchOn(TRUE);
   }
}

/**
 * redirect the signal for insertion of buttons to an own slot
 * that means: If the menubar (where the buttons should be inserted) is given,
 *             QextMDI can insert them automatically.
 *             Otherwise only signals can be emitted to tell the outside that
 *             someone must do this job itself.
 */
#ifndef NO_KDE
void QextMdiMainFrm::setMenuForSDIModeSysButtons( KMenuBar* pMenuBar)
#else
void QextMdiMainFrm::setMenuForSDIModeSysButtons( QMenuBar* pMenuBar)
#endif
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
   m_pUndock->setAutoRaise(FALSE);
   m_pMinimize->setAutoRaise(FALSE);
   m_pRestore->setAutoRaise(FALSE);
   m_pClose->setAutoRaise(FALSE);

   setSysButtonsAtMenuPosition();

   delete m_pUndockButtonPixmap;
   delete m_pMinButtonPixmap;
   delete m_pRestoreButtonPixmap;
   delete m_pCloseButtonPixmap;
   // create the decoration pixmaps
   if (frameDecorOfAttachedViews() == QextMdi::Win95Look) {
      m_pUndockButtonPixmap = new QPixmap( win_undockbutton);
      m_pMinButtonPixmap = new QPixmap( win_minbutton);
      m_pRestoreButtonPixmap = new QPixmap( win_restorebutton);
      m_pCloseButtonPixmap = new QPixmap( win_closebutton);
   }
   else if (frameDecorOfAttachedViews() == QextMdi::KDE1Look) {
      m_pUndockButtonPixmap = new QPixmap( kde_undockbutton);
      m_pMinButtonPixmap = new QPixmap( kde_minbutton);
      m_pRestoreButtonPixmap = new QPixmap( kde_restorebutton);
      m_pCloseButtonPixmap = new QPixmap( kde_closebutton);
      m_pUndock->setAutoRaise(TRUE);
      m_pMinimize->setAutoRaise(TRUE);
      m_pRestore->setAutoRaise(TRUE);
      m_pClose->setAutoRaise(TRUE);
   }
   else if (frameDecorOfAttachedViews() == QextMdi::KDELook) {
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

void QextMdiMainFrm::setSysButtonsAtMenuPosition()
{
   if( m_pMainMenuBar == 0L)
      return;
   if( m_pMainMenuBar->parentWidget() == 0L)
      return;

   int menuW = m_pMainMenuBar->parentWidget()->width();
   int h;
   int y;
   if (frameDecorOfAttachedViews() == QextMdi::Win95Look)
      h = 16;
   else if (frameDecorOfAttachedViews() == QextMdi::KDE1Look)
      h = 20;
   else if (frameDecorOfAttachedViews() == QextMdi::KDELook)
      h = 16;
   else
      h = 14;
   y = m_pMainMenuBar->height()/2 - h/2;

   if (frameDecorOfAttachedViews() == QextMdi::KDELaptopLook) {
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
void QextMdiMainFrm::activateNextWin()
{
   QextMdiIterator<QextMdiChildView*>* it = createIterator();
   QextMdiChildView* aWin = activeWindow();
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
void QextMdiMainFrm::activatePrevWin()
{
   QextMdiIterator<QextMdiChildView*>* it = createIterator();
   QextMdiChildView* aWin = activeWindow();
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
void QextMdiMainFrm::activateFirstWin()
{
   QextMdiIterator<QextMdiChildView*>* it = createIterator();
   QMap<QDateTime,QextMdiChildView*> m;
   for (it->first(); !it->isDone(); it->next()) {
      m.insert(it->currentItem()->getTimeStamp(), it->currentItem());
   }

   QDateTime current = activeWindow()->getTimeStamp();
   QMap<QDateTime,QextMdiChildView*>::iterator pos(m.find(current));
   QMap<QDateTime,QextMdiChildView*>::iterator newPos = pos;
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
   m_bSwitching= TRUE; // flag that we are currently switching between windows
   delete it;
}

/** Activates the previously accessed view before this one was activated */
void QextMdiMainFrm::activateLastWin()
{
   QextMdiIterator<QextMdiChildView*>* it = createIterator();
   QMap<QDateTime,QextMdiChildView*> m;
   for (it->first(); !it->isDone(); it->next()) {
      m.insert(it->currentItem()->getTimeStamp(), it->currentItem());
   }

   QDateTime current = activeWindow()->getTimeStamp();
   QMap<QDateTime,QextMdiChildView*>::iterator pos(m.find(current));
   if (pos != m.begin()) {
      --pos;
   }
   else {
      pos = m.end();
      --pos;
   }
   activateView(pos.data());
   m_bSwitching= TRUE; // flag that we are currently switching between windows
   delete it;
}

/** Activates the view with a certain index (TabPage mode only) */
void QextMdiMainFrm::activateView(int index)
{
   QextMdiChildView* pView = m_pWinList->first();
   for (int i = 0; pView && (i < index); i++) {
      pView = m_pWinList->next();
   }
   if (pView) {
      pView->activate();
   }
}

/** turns the system buttons for maximize mode (SDI mode) on, and connects them with the current child frame */
void QextMdiMainFrm::setEnableMaximizedChildFrmMode(bool bEnable)
{
   if (bEnable) {
      m_bMaximizedChildFrmMode = TRUE;
      //qDebug("MaximizeMode on");

      QextMdiChildFrm* pCurrentChild = m_pMdi->topChild();
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

      if (frameDecorOfAttachedViews() == QextMdi::KDELaptopLook) {
         m_pMainMenuBar->insertItem( QPixmap(kde2laptop_closebutton_menu), m_pMdi->topChild(), SLOT(closePressed()), 0, -1, 0);
      }
      else {
         m_pMainMenuBar->insertItem( *pCurrentChild->icon(), pCurrentChild->systemMenu(), -1, 0);
         QObject::connect( m_pClose, SIGNAL(clicked()), pCurrentChild, SLOT(closePressed()) );
         m_pClose->show();
      }
   }
   else {
      if (!m_bMaximizedChildFrmMode) return;  // already set, nothing to do

      m_bMaximizedChildFrmMode = FALSE;
      //qDebug("MaximizeMode off");

      QextMdiChildFrm* pFrmChild = m_pMdi->topChild();
      if (!pFrmChild) return;

      if (pFrmChild->m_pClient && pFrmChild->state() == QextMdiChildFrm::Maximized) {
         pFrmChild->m_pClient->restore();
         switchOffMaximizeModeForMenu( pFrmChild);
      }
   }
}

/** turns the system buttons for maximize mode (SDI mode) off, and disconnects them */
void QextMdiMainFrm::switchOffMaximizeModeForMenu(QextMdiChildFrm* oldChild)
{
   //qDebug("switching off maximize mode for menu");

   // if there is no menubar given, those system buttons aren't possible
   if( m_pMainMenuBar == 0L)
      return;
      
   m_pMainMenuBar->removeItem( m_pMainMenuBar->idAt(0));

   if( oldChild) {
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
void QextMdiMainFrm::updateSysButtonConnections( QextMdiChildFrm* oldChild, QextMdiChildFrm* newChild)
{
   //qDebug("updateSysButtonConnections");
   // if there is no menubar given, those system buttons aren't possible
   if( m_pMainMenuBar == 0L)
      return;
      
   if (newChild) {
      if (frameDecorOfAttachedViews() == QextMdi::KDELaptopLook) {
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
      QObject::disconnect( m_pUndock, SIGNAL(clicked()), oldChild, SLOT(undockPressed()) );
      QObject::disconnect( m_pMinimize, SIGNAL(clicked()), oldChild, SLOT(minimizePressed()) );
      QObject::disconnect( m_pRestore, SIGNAL(clicked()), oldChild, SLOT(maximizePressed()) );
      QObject::disconnect( m_pClose, SIGNAL(clicked()), oldChild, SLOT(closePressed()) );
   }
   if (newChild) {
      QObject::connect( m_pUndock, SIGNAL(clicked()), newChild, SLOT(undockPressed()) );
      QObject::connect( m_pMinimize, SIGNAL(clicked()), newChild, SLOT(minimizePressed()) );
      QObject::connect( m_pRestore, SIGNAL(clicked()), newChild, SLOT(maximizePressed()) );
      QObject::connect( m_pClose, SIGNAL(clicked()), newChild, SLOT(closePressed()) );
   }
}

/** Shows the view taskbar. This should be connected with your "View" menu. */
bool QextMdiMainFrm::isViewTaskBarOn()
{
   bool bOn = false;
   if (m_pTaskBar)
      bOn = m_pTaskBar->isSwitchedOn();
   return bOn;
}

/** Shows the view taskbar. This should be connected with your "View" menu. */
void QextMdiMainFrm::showViewTaskBar()
{
   if (m_pTaskBar)
      m_pTaskBar->switchOn(TRUE);
}

/** Hides the view taskbar. This should be connected with your "View" menu. */
void QextMdiMainFrm::hideViewTaskBar()
{
   if (m_pTaskBar)
      m_pTaskBar->switchOn(FALSE);
}

//=============== fillWindowMenu ===============//
void QextMdiMainFrm::fillWindowMenu()
{
   bool bTabPageMode = FALSE;
   if (m_mdiMode == QextMdi::TabPageMode)
      bTabPageMode = TRUE;

   bool bNoViewOpened = FALSE;
   if (m_pWinList->isEmpty()) {
      bNoViewOpened = TRUE;
   }
   // construct the menu and its submenus
   if (!m_bClearingOfWindowMenuBlocked) {
      m_pWindowMenu->clear();
   }
   int closeId = m_pWindowMenu->insertItem(tr("&Close"), this, SLOT(closeActiveView()));
   int closeAllId = m_pWindowMenu->insertItem(tr("Close &All"), this, SLOT(closeAllViews()));
   if (bNoViewOpened) {
      m_pWindowMenu->setItemEnabled(closeId, FALSE);
      m_pWindowMenu->setItemEnabled(closeAllId, FALSE);
   }
   if (!bTabPageMode) {
      int iconifyId = m_pWindowMenu->insertItem(tr("&Minimize All"), this, SLOT(iconifyAllViews()));
      if (bNoViewOpened) {
         m_pWindowMenu->setItemEnabled(iconifyId, FALSE);
      }
   }
   m_pWindowMenu->insertSeparator();
   m_pWindowMenu->insertItem(tr("&MDI Mode..."), m_pMdiModeMenu);
   m_pMdiModeMenu->clear();
   m_pMdiModeMenu->insertItem(tr("&Toplevel mode"), this, SLOT(switchToToplevelMode()));
   m_pMdiModeMenu->insertItem(tr("C&hildframe mode"), this, SLOT(switchToChildframeMode()));
   m_pMdiModeMenu->insertItem(tr("Ta&b Page mode"), this, SLOT(switchToTabPageMode()));
   switch (m_mdiMode) {
   case QextMdi::ToplevelMode:
      m_pMdiModeMenu->setItemChecked(m_pMdiModeMenu->idAt(0), TRUE);
      break;
   case QextMdi::ChildframeMode:
      m_pMdiModeMenu->setItemChecked(m_pMdiModeMenu->idAt(1), TRUE);
      break;
   case QextMdi::TabPageMode:
      m_pMdiModeMenu->setItemChecked(m_pMdiModeMenu->idAt(2), TRUE);
      break;
   default:
      break;
   }
   m_pWindowMenu->insertSeparator();
   if (!bTabPageMode) {
      int placMenuId = m_pWindowMenu->insertItem(tr("&Tile..."), m_pPlacingMenu);
      m_pPlacingMenu->clear();
      m_pPlacingMenu->insertItem(tr("Ca&scade windows"), m_pMdi,SLOT(cascadeWindows()));
      m_pPlacingMenu->insertItem(tr("Cascade &maximized"), m_pMdi,SLOT(cascadeMaximized()));
      m_pPlacingMenu->insertItem(tr("Expand &vertically"), m_pMdi,SLOT(expandVertical()));
      m_pPlacingMenu->insertItem(tr("Expand &horizontally"), m_pMdi,SLOT(expandHorizontal()));
      m_pPlacingMenu->insertItem(tr("Tile &non-overlapped"), m_pMdi,SLOT(tileAnodine()));
      m_pPlacingMenu->insertItem(tr("Tile overla&pped"), m_pMdi,SLOT(tilePragma()));
      m_pPlacingMenu->insertItem(tr("Tile v&ertically"), m_pMdi,SLOT(tileVertically()));
      if (m_mdiMode == QextMdi::ToplevelMode) {
         m_pWindowMenu->setItemEnabled(placMenuId, FALSE);
      }
      m_pWindowMenu->insertSeparator();
      int dockUndockId = m_pWindowMenu->insertItem(tr("&Dock/Undock..."), m_pDockMenu);
         m_pDockMenu->clear();
      m_pWindowMenu->insertSeparator();
      if (bNoViewOpened) {
         m_pWindowMenu->setItemEnabled(placMenuId, FALSE);
         m_pWindowMenu->setItemEnabled(dockUndockId, FALSE);
      }
   }
   int entryCount = m_pWindowMenu->count();

   // for all child frame windows: give an ID to every window and connect them in the end with windowMenuItemActivated()
   int i=100;
   QextMdiChildView* pView = 0L;
   QPtrListIterator<QextMdiChildView> it(*m_pWinList);
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
      bool inserted = FALSE;
      QString tmpString;
      QValueList<QDateTime>::iterator timeStampIterator = timeStamps.begin();
      for (indx = 0; indx <= windowItemCount; indx++, ++timeStampIterator) {
        bool putHere = FALSE;
        if ((*timeStampIterator) < timeStamp) {
          putHere = TRUE;
          timeStamps.insert(timeStampIterator, timeStamp);
        }
        if (putHere) {
            m_pWindowMenu->insertItem( item, pView, SLOT(slot_clickedInWindowMenu()), 0, -1, indx+entryCount);
            if (pView == m_pCurrentWindow) {
               m_pWindowMenu->setItemChecked( m_pWindowMenu->idAt( indx+entryCount), TRUE);
            }
            pView->setWindowMenuID(i);
            if (!bTabPageMode) {
               m_pDockMenu->insertItem( item, pView, SLOT(slot_clickedInDockMenu()), 0, -1, indx);
               if (pView->isAttached()) {
                  m_pDockMenu->setItemChecked( m_pDockMenu->idAt( indx), TRUE);
               }
            }
            inserted = TRUE;
            break;
            indx = windowItemCount+1;  // break the loop
         }
      }
      if (!inserted) {  // append it
         m_pWindowMenu->insertItem( item, pView, SLOT(slot_clickedInWindowMenu()), 0, -1, windowItemCount+entryCount);
         if (pView == m_pCurrentWindow) {
            m_pWindowMenu->setItemChecked( m_pWindowMenu->idAt(windowItemCount+entryCount), TRUE);
         }
         pView->setWindowMenuID( i);
         if (!bTabPageMode) {
            m_pDockMenu->insertItem( item, pView, SLOT(slot_clickedInDockMenu()), 0, -1, windowItemCount);
            if (pView->isAttached()) {
               m_pDockMenu->setItemChecked( m_pDockMenu->idAt(windowItemCount), TRUE);
            }
         }
      }
      i++;
   }
}

//================ windowMenuItemActivated ===============//

void QextMdiMainFrm::windowMenuItemActivated(int id)
{
   if (id < 100) return;
   id -= 100;
   QextMdiChildView *pView = m_pWinList->at( id);
   if (!pView) return;
   if (pView->isMinimized()) pView->minimize();
   if (m_mdiMode != QextMdi::TabPageMode) {
      QextMdiChildFrm* pTopChild = m_pMdi->topChild();
      if (pTopChild) {
         if ((pView == pTopChild->m_pClient) && pView->isAttached()) {
            return;
         }
      }
   }
   activateView( pView);
}

//================ dockMenuItemActivated ===============//

void QextMdiMainFrm::dockMenuItemActivated(int id)
{
   if( id < 100) return;
   id -= 100;
   QextMdiChildView *pView = m_pWinList->at( id);
   if( !pView) return;
   if( pView->isMinimized()) pView->minimize();
   if( pView->isAttached()) {
      detachWindow( pView, TRUE);
   }
   else {   // is detached
      attachWindow( pView, TRUE);
   }
}

//================ popupWindowMenu ===============//

void QextMdiMainFrm::popupWindowMenu(QPoint p)
{
   if (!isFakingSDIApplication()) {
      m_pWindowMenu->popup( p);
   }
}

//================ dragEndTimeOut ===============//
void QextMdiMainFrm::dragEndTimeOut()
{
   // send drag end to all concerned views.
   QextMdiChildView* pView;
   for (m_pWinList->first(); (pView = m_pWinList->current()) != 0L; m_pWinList->next()) {
      QextMdiChildFrmDragEndEvent   dragEndEvent(0L);
      QApplication::sendEvent(pView, &dragEndEvent);
   }
}

//================ setFrameDecorOfAttachedViews ===============//

void QextMdiMainFrm::setFrameDecorOfAttachedViews( int frameDecor)
{
   switch (frameDecor) {
   case 0:
      m_frameDecoration = QextMdi::Win95Look;
      break;
   case 1:
      m_frameDecoration = QextMdi::KDE1Look;
      break;
   case 2:
      m_frameDecoration = QextMdi::KDELook;
      break;
   case 3:
      m_frameDecoration = QextMdi::KDELaptopLook;
      break;
   default:
      qDebug("unknown MDI decoration");
      break;
   }
   setMenuForSDIModeSysButtons( m_pMainMenuBar);
   QPtrListIterator<QextMdiChildView> it( *m_pWinList);
   for( ; it.current(); ++it) {
      QextMdiChildView* pView = it.current();
      if( pView->isToolView())
         continue;
      if( pView->isAttached())
         pView->mdiParent()->redecorateButtons();
   }
}

void QextMdiMainFrm::fakeSDIApplication()
{
   m_bSDIApplication = TRUE;
   if (m_pTaskBar)
      m_pTaskBar->close();
   m_pTaskBar = 0L;
};

void QextMdiMainFrm::closeViewButtonPressed()
{
   QextMdiChildView* pView = activeWindow();
   if (pView) {
      pView->close();
   }
}

#ifndef NO_INCLUDE_MOCFILES
#include "qextmdimainfrm.moc"
#endif
