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
#ifndef NO_KDE2
#include <kmenubar.h>
#endif
#include <qtoolbutton.h>
#include <qlayout.h>

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

using namespace KParts;

#ifdef _OS_WIN32_
QextMdi::FrameDecor QextMdiMainFrm::m_frameDecoration = QextMdi::Win95Look;
#else
#ifdef NO_KDE2
QextMdi::FrameDecor QextMdiMainFrm::m_frameDecoration = QextMdi::KDE1Look;
#else
QextMdi::FrameDecor QextMdiMainFrm::m_frameDecoration = QextMdi::KDE2Look;
#endif
#endif

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
   ,m_mdiMode(QextMdi::ChildframeMode)
   ,m_bMaximizedChildFrmMode(FALSE)
   ,m_oldMainFrmHeight(0)
   ,m_oldMainFrmMinHeight(0)
   ,m_oldMainFrmMaxHeight(0)
   ,m_bSDIApplication(false)
   ,m_pDockbaseAreaOfDocumentViews(0L)
   ,m_pDockbaseOfTabPage(0L)
   ,m_pTempDockSession(0L)
   ,m_bClearingOfWindowMenuBlocked(FALSE)
{
   // Create the local list of windows
   m_pWinList = new QList<QextMdiChildView>;
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
}

//============ ~QextMdiMainFrm ============//
QextMdiMainFrm::~QextMdiMainFrm()
{
   // safely close the windows so properties are saved...
   QextMdiChildView *pWnd = 0L;
   while((pWnd = m_pWinList->first()))closeWindow(pWnd, FALSE); // without re-layout taskbar!
   emit lastChildViewClosed();
   delete m_pWinList;
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
}

//============ createTaskBar ==============//
void QextMdiMainFrm::createTaskBar()
{
   m_pTaskBar = new QextMdiTaskBar(this,QMainWindow::Bottom);
   m_pTaskBar->installEventFilter( this);
}

void QextMdiMainFrm::slot_toggleTaskBar()
{
   if (!m_pTaskBar)
      return;
   if (m_pTaskBar->isVisible()){
      m_pTaskBar->hide();
   } else {
      m_pTaskBar->show();
   }
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
      KDockWidget* pCover = createDockWidget( pWnd->name(),
                                              pWnd->icon() ? *(pWnd->icon()) : QPixmap(),
                                              0L,  // parent
                                              pWnd->caption(),
                                              pWnd->tabCaption());
      pCover->setWidget( pWnd);
      pCover->setToolTipString( pWnd->caption());
      if (!(flags & QextMdi::Detach)) {
         m_pDockbaseOfTabPage->setDockSite(KDockWidget::DockFullSite);
         pCover->manualDock( m_pDockbaseOfTabPage, KDockWidget::DockCenter);
         pCover->setEnableDocking(KDockWidget::DockNone);
         if (m_pDockbaseOfTabPage == m_pDockbaseAreaOfDocumentViews)
            m_pDockbaseAreaOfDocumentViews->undock();
         m_pDockbaseOfTabPage->setDockSite(KDockWidget::DockCorner);
         m_pDockbaseOfTabPage = pCover;
      }
      pCover->show();
      pWnd->setFocus();
   }
   else {
      if( (flags & QextMdi::Detach) || (m_mdiMode == QextMdi::ToplevelMode)) {
         detachWindow( pWnd, !(flags & QextMdi::Hide));
      } else {
         attachWindow( pWnd, !(flags & QextMdi::Hide));
      }

      if ( m_bMaximizedChildFrmMode || (flags & QextMdi::Maximize) || (m_bSDIApplication && !(flags & QextMdi::Detach)) ) {
         if (!pWnd->isMaximized())
            pWnd->maximize();
      }
      if (!m_bSDIApplication || (flags & QextMdi::Detach)) {
         if (flags & QextMdi::Minimize)
            pWnd->minimize();
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
   pWnd->setGeometry( rectNormal);
}

//============ addWindow ============//
void QextMdiMainFrm::addWindow( QextMdiChildView* pWnd, QPoint pos, int flags)
{
   addWindow( pWnd, flags);
   pWnd->move( pos);
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
      pToolView->setIcon( pWnd->icon() ? *(pWnd->icon()) : QPixmap());
      pToolView->setCaption( pWnd->caption());
      QApplication::sendPostedEvents();
      pLayout->addWidget( pWnd);
   }

   // if docking is not desired, add the toolview as stay-on-top toplevel view
   if (pos == KDockWidget::DockNone) {
      pToolView->reparent(this, WType_TopLevel | WStyle_Dialog, r.topLeft(), pToolView->isVisible());
      QObject::connect( pToolView, SIGNAL(childWindowCloseRequest(QextMdiChildView*)), this, SLOT(childWindowCloseRequest(QextMdiChildView*)) );
      m_pWinList->append(pToolView);
      pToolView->m_bToolView = TRUE;
      pToolView->setGeometry(r);
      pToolView->show();
   }
   else {   // add (and dock) the toolview as DockWidget view
      KDockWidget* pCover = createDockWidget( pToolView->name(),
                                              pToolView->icon() ? *(pToolView->icon()) : QPixmap(),
                                              0L,  // parent
                                              pToolView->caption(),
                                              tabCaption.isEmpty() ? pToolView->tabCaption() : tabCaption);
      pCover->setWidget( pToolView);
      pCover->setToolTipString( tabToolTip);
      KDockWidget* pTargetDock = 0L;
      if ((pTargetWnd == m_pDockbaseAreaOfDocumentViews->getWidget()) || (pTargetWnd == this)) {
         pTargetDock = m_pDockbaseAreaOfDocumentViews;
      }
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
      pCover->show();
   }
}

//============ attachWindow ============//
void QextMdiMainFrm::attachWindow(QextMdiChildView *pWnd, bool bShow)
{
   pWnd->installEventFilter(this);

   // decide whether window shall be cascaded
   bool bCascade = FALSE;
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
   lpC->setClient(pWnd);
   lpC->setFocus();
   pWnd->youAreAttached(lpC);
   if( (m_mdiMode == QextMdi::ToplevelMode) && !parentWidget()) {
      setMinimumHeight( m_oldMainFrmMinHeight);
      setMaximumHeight( m_oldMainFrmMaxHeight);
      resize( width(), m_oldMainFrmHeight);
      m_oldMainFrmHeight = 0;
      m_mdiMode = QextMdi::ChildframeMode;
      //qDebug("TopLevelMode off");
      emit leftTopLevelMode();
   }

   m_pMdi->manageChild(lpC,FALSE,bCascade);

   if (m_bMaximizedChildFrmMode && (m_pMdi->m_pZ->count() > 1)) {
//     updateSysButtonConnections( m_pMdi->topChild(), lpC);
   }

   if (bShow) {
      lpC->show();
   }

   m_pCurrentWindow  = pWnd;  // required for checking the active item
   QFocusEvent *fe = new QFocusEvent(QEvent::FocusIn);
   QApplication::sendEvent( pWnd, fe);
}

//============= detachWindow ==============//
void QextMdiMainFrm::detachWindow(QextMdiChildView *pWnd, bool bShow)
{
   if (!pWnd->isAttached()) return;

   pWnd->removeEventFilter(this);
   pWnd->youAreDetached();
   // this is only if it was attached and you want to detach it
   if(pWnd->parent() != NULL ) {
      QextMdiChildFrm *lpC=pWnd->mdiParent();
      if (lpC) {
        lpC->unsetClient( m_undockPositioningOffset);
        m_pMdi->destroyChildButNotItsView(lpC,FALSE); //Do not focus the new top child , we loose focus...
      }
   }
   else {
      if( pWnd->geometry() == QRect(0,0,1,1)) {
         pWnd->setGeometry( QRect( m_pMdi->getCascadePoint(m_pWinList->count()-1), defaultChildFrmSize()));
      }
   }

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

   if(pWnd->isAttached()) {
      pWnd->mdiParent()->hide();
      m_pMdi->destroyChildButNotItsView(pWnd->mdiParent());
   }

   if(pWnd->isToolView())
      pWnd->m_bToolView = FALSE;
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

   if(pWnd->isAttached())
      m_pMdi->destroyChild(pWnd->mdiParent());
   else
      delete pWnd;

   if( !m_pCurrentWindow)
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
QPopupMenu * QextMdiMainFrm::taskBarPopup(QextMdiChildView *pWnd,bool bIncludeWindowPopup)
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
   bIncludeWindowPopup = FALSE; // dummy!, only to avoid "unused parameter"
}

void QextMdiMainFrm::activateView(QextMdiChildView *pWnd)
{
   m_pCurrentWindow = pWnd;
   if (m_pTaskBar) {
      m_pTaskBar->setActiveButton(pWnd);
   }

   if (m_mdiMode == QextMdi::TabPageMode) {
      makeWidgetDockVisible(pWnd);
   }
   else {
      if (pWnd->isAttached()){
         pWnd->mdiParent()->raiseAndActivate();
      }
      else {
         if (!pWnd->hasFocus() || !pWnd->isActiveWindow()) {
            pWnd->show();
            pWnd->setActiveWindow();
            pWnd->raise();
         }
         m_pMdi->setTopChild(0L); // lose focus in the mainframe window
      }
      pWnd->setFocus();
   }
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

void QextMdiMainFrm::focusInEvent(QFocusEvent *)
{
   //qDebug("QextMdiMainFrm::focusInEvent");
   m_pMdi->setFocus();
}

bool QextMdiMainFrm::event( QEvent* e)
{
   if( e->type() == QEvent::FocusIn) {
      qDebug("FocusIn");
   }
   if( e->type() == QEvent::User) {
      QextMdiChildView* pWnd = (QextMdiChildView*)((QextMdiViewCloseEvent*)e)->data();
      if( pWnd != 0L)
         closeWindow( pWnd);
      return TRUE;
   }

   return DockMainWindow::event( e);
}

bool QextMdiMainFrm::eventFilter(QObject *obj, QEvent *e )
{
   if( e->type() == QEvent::FocusIn) {
      QFocusEvent* pFE = (QFocusEvent*) e;
      if (pFE->reason() == QFocusEvent::ActiveWindow) {
         if (!m_pCurrentWindow->isAttached() && (m_pMdi->topChild() != 0L)) {
            return TRUE;   // eat the event
         }
      }
   }
   return DockMainWindow::eventFilter( obj, e);
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

/**
 * undocks all view windows (unix-like)
 */
void QextMdiMainFrm::switchToToplevelMode()
{
   if (m_mdiMode == QextMdi::ToplevelMode)
      return;

   // save the old dock szenario of the dockwidged-like tool views to a DOM tree
   delete m_pTempDockSession;
   m_pTempDockSession = new QDomDocument( "docksession");
   QDomElement curDockState = m_pTempDockSession->createElement("cur_dock_state");
   m_pTempDockSession->appendChild( curDockState);
   writeDockConfig( curDockState);

   QextMdi::MdiMode oldMdiMode = m_mdiMode;

   // since we set some windows to toplevel, we must consider the window manager's window frame
   const int frameBorderWidth  = 7;  // TODO: Can we / do we need to ask the window manager?
   const int windowTitleHeight = 10; // TODO:    -"-
   setUndockPositioningOffset( QPoint( 0, (m_pTaskBar ? m_pTaskBar->height() : 0) + frameBorderWidth));

   // 1.) select the dockwidgets to be undocked and store their geometry
   QObjectList* pObjList = queryList( "KDockWidget");
   QObjectListIt it( *pObjList);
   QObject* pObj;
   QValueList<QRect> positionList;
   QList<KDockWidget> rootDockWidgetList;
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
           QListIterator<KDockWidget> it2( rootDockWidgetList);
           if (!rootDockWidgetList.isEmpty()) {
               for ( ; it2.current() && !found; ++it2 ) {
                   KDockWidget* pDockW = it2.current();
                   if (pDockW == pRootDockW)
                       found = TRUE;
               }
               if (!found) {
                   rootDockWidgetList.append( pDockW);
                   QPoint p = pDockW->mapToGlobal( pDockW->pos())-pDockW->pos();
                   QRect r( p.x(),
                            p.y()+m_undockPositioningOffset.y(),
                            pDockW->width()  - windowTitleHeight - frameBorderWidth*2,
                            pDockW->height() - windowTitleHeight - frameBorderWidth*2);
                   positionList.append( r);
               }
           }
           else {
               rootDockWidgetList.append( pRootDockW);
               QPoint p = pRootDockW->mapToGlobal( pRootDockW->pos())-pRootDockW->pos();
               QRect r( p.x(),
                        p.y()+m_undockPositioningOffset.y(),
                        pRootDockW->width()  - windowTitleHeight - frameBorderWidth*2,
                        pRootDockW->height() - windowTitleHeight - frameBorderWidth*2);
               positionList.append( r);
           }
       }
   }

   // 2.) undock the MDI views of QextMDI
   if (oldMdiMode == QextMdi::ChildframeMode) {
      finishChildframeMode();
   }
   else if (oldMdiMode == QextMdi::TabPageMode) { // if tabified, release all views from their docking covers
      finishTabPageMode();
   }

   // 3.) undock all these found oldest ancestors (being KDockWidgets)
   QListIterator<KDockWidget> it3( rootDockWidgetList);
   for (; it3.current(); ++it3 ) {
       KDockWidget* pDockW = it3.current();
       pDockW->undock();
   }

   // 4.) recreate the MDI childframe area and hide it
   if (oldMdiMode == QextMdi::TabPageMode) {
      QApplication::sendPostedEvents();
      m_pDockbaseAreaOfDocumentViews = createDockWidget( "mdiAreaCover", QPixmap(), 0L, "mdi_area_cover");
      m_pDockbaseAreaOfDocumentViews->setEnableDocking(KDockWidget::DockNone);
      m_pDockbaseAreaOfDocumentViews->setDockSite(KDockWidget::DockCorner);
      m_pDockbaseAreaOfDocumentViews->setWidget(m_pMdi);
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

   // 5.) reset all memorized positions of the undocked ones and show them again
   QValueList<QRect>::Iterator it5;
   for (it3.toFirst(), it5 = positionList.begin() ; it3.current(), it5 != positionList.end(); ++it3, ++it5 ) {
       KDockWidget* pDockW = it3.current();
       pDockW->setGeometry( (*it5));
       pDockW->show();
   }

   m_mdiMode = QextMdi::ToplevelMode;
   //qDebug("ToplevelMode on");
}

void QextMdiMainFrm::finishToplevelMode()
{
}

/**
 * docks all view windows (Windows-like)
 */
void QextMdiMainFrm::switchToChildframeMode()
{
   if (m_mdiMode == QextMdi::ChildframeMode)
      return;

   if (m_mdiMode == QextMdi::TabPageMode) {
      finishTabPageMode();
   }

   if (m_pDockbaseAreaOfDocumentViews == 0L) {
      // cover QextMdi's childarea by a dockwidget
      m_pDockbaseAreaOfDocumentViews = createDockWidget( "mdiAreaCover", QPixmap(), 0L, "mdi_area_cover");
      m_pDockbaseAreaOfDocumentViews->setEnableDocking(KDockWidget::DockNone);
      m_pDockbaseAreaOfDocumentViews->setDockSite(KDockWidget::DockCorner);
      m_pDockbaseAreaOfDocumentViews->setWidget(m_pMdi);
      // set this dock to main view
      setView(m_pDockbaseAreaOfDocumentViews);
      setMainDockWidget(m_pDockbaseAreaOfDocumentViews);
   }

//   if (m_mdiMode == QextMdi::TabPageMode) {
//      finishTabPageMode();
//      // switch the tab-page dockbase and the childarea dockbase
//      m_pDockbaseOfTabPage->setEnableDocking(KDockWidget::DockFullDocking);
//      m_pDockbaseAreaOfDocumentViews->setEnableDocking(KDockWidget::DockCenter);
//      m_pDockbaseAreaOfDocumentViews->manualDock( m_pDockbaseOfTabPage, KDockWidget::DockCenter);
//      m_pDockbaseAreaOfDocumentViews->makeDockVisible();
//      m_pDockbaseOfTabPage->undock();
//      m_pDockbaseAreaOfDocumentViews->setDockSite(KDockWidget::DockCorner);
//      m_pDockbaseAreaOfDocumentViews->setEnableDocking(KDockWidget::DockNone);
//   }

   if (m_mdiMode == QextMdi::ToplevelMode) {
     // restore the old dock szenario which we memorized at the time we switched to toplevel mode
     QDomElement oldDockState = m_pTempDockSession->namedItem("cur_dock_state").toElement();
     readDockConfig( oldDockState);
   }

   QListIterator<QextMdiChildView> it( *m_pWinList);
   for( ; it.current(); ++it) {
      QextMdiChildView* pView = it.current();
      if( !pView->isToolView())
         if( !pView->isAttached())
            attachWindow( pView, TRUE);
   }
   if( (m_mdiMode == QextMdi::ToplevelMode) && !parentWidget()) {
      setMinimumHeight( m_oldMainFrmMinHeight);
      setMaximumHeight( m_oldMainFrmMaxHeight);
      resize( width(), m_oldMainFrmHeight);
      m_oldMainFrmHeight = 0;
      //qDebug("TopLevelMode off");
      emit leftTopLevelMode();
   }

   // the new MDI mode is set at last
   m_mdiMode = QextMdi::ChildframeMode;
}

void QextMdiMainFrm::finishChildframeMode()
{
   QListIterator<QextMdiChildView> it4( *m_pWinList);
   for( ; it4.current(); ++it4) {
      QextMdiChildView* pView = it4.current();
      if( pView->isToolView())
         continue;
      if( pView->isAttached()) {
         if( pView->isMaximized())
            pView->mdiParent()->setGeometry( 0, 0, m_pMdi->width(), m_pMdi->height());
         detachWindow( pView, TRUE);
      }
   }
   // alternative?:
//     QextMdiIterator<QextMdiChildView*>* pItMdi = createIterator();
//     for (pItMdi->first(); !pItMdi->isDone(); pItMdi->next()) {
//         pItMdi->currentItem()->detach();
//     }
//     delete pItMdi;
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
   QListIterator<QextMdiChildView> it4( *m_pWinList);
   for( ; it4.current(); ++it4) {
      QextMdiChildView* pView = it4.current();
      if( pView->isToolView())
         continue;
      pCover = createDockWidget( pView->name(),
                                 pView->icon() ? *(pView->icon()) : QPixmap(),
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
      // set the first page as active page
      KDockTabCtl* pTab = (KDockTabCtl*) pCover->parentWidget()->parentWidget();
      pTab->setVisiblePage(pRemActiveWindow);
      pRemActiveWindow->setFocus();
   }

   m_pTaskBar->hide();
   //qDebug("TabPageMode on");
}

void QextMdiMainFrm::finishTabPageMode()
{
   // if tabified, release all views from their docking covers
   if (m_mdiMode == QextMdi::TabPageMode) {
      QListIterator<QextMdiChildView> it4( *m_pWinList);
      for( ; it4.current(); ++it4) {
         QextMdiChildView* pView = it4.current();
         if( pView->isToolView())
            continue;
         QSize mins = pView->minimumSize();
         QSize maxs = pView->maximumSize();
         QSize sz = pView->size();
         QWidget* pParent = pView->parentWidget();
         pView->reparent(0,0,pParent->mapToGlobal(pParent->pos())-pParent->pos()+m_undockPositioningOffset,TRUE);
         pView->resize(sz);
         pView->setMinimumSize(mins.width(),mins.height());
         pView->setMaximumSize(maxs.width(),maxs.height());
         ((KDockWidget*)pParent)->undock(); // this destroys the dockwiget cover, too
         pParent->close();
         delete pParent;
      }
      m_pTaskBar->show();
   }
}

/**
 * redirect the signal for insertion of buttons to an own slot
 * that means: If the menubar (where the buttons should be inserted) is given,
 *             QextMDI can insert them automatically.
 *             Otherwise only signals can be emitted to tell the outside that
 *             someone must do this job itself.
 */
#ifndef NO_KDE2
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
#if QT_VERSION > 209
      m_pUndock->setAutoRaise(TRUE);
      m_pMinimize->setAutoRaise(TRUE);
      m_pRestore->setAutoRaise(TRUE);
      m_pClose->setAutoRaise(TRUE);
#endif
   }
   else if (frameDecorOfAttachedViews() == QextMdi::KDE2Look) {
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
   else if (frameDecorOfAttachedViews() == QextMdi::KDE2Look)
      h = 16;
   else
      h = 14;
   y = m_pMainMenuBar->height()/2 - h/2;

   if (frameDecorOfAttachedViews() == QextMdi::KDE2LaptopLook) {
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

      if (frameDecorOfAttachedViews() == QextMdi::KDE2LaptopLook) {
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
void QextMdiMainFrm::showViewTaskBar()
{
   if (m_pTaskBar)
      m_pTaskBar->show();
}

/** Hides the view taskbar. This should be connected with your "View" menu. */
void QextMdiMainFrm::hideViewTaskBar()
{
   if (m_pTaskBar)
      m_pTaskBar->hide();
}

//=============== fillWindowMenu ===============//
void QextMdiMainFrm::fillWindowMenu()
{
   bool bTabPageMode = FALSE;
   if (m_mdiMode == QextMdi::TabPageMode)
      bTabPageMode = TRUE;

   // construct the menu and its submenus
   if (!m_bClearingOfWindowMenuBlocked) {
      m_pWindowMenu->clear();
   }
   m_pWindowMenu->insertItem(tr("&Close"), this, SLOT(closeActiveView()));
   m_pWindowMenu->insertItem(tr("Close &All"), this, SLOT(closeAllViews()));
   if (!bTabPageMode) {
      m_pWindowMenu->insertItem(tr("&Iconify All"), this, SLOT(iconifyAllViews()));
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
      m_pWindowMenu->insertItem(tr("&Placing..."), m_pPlacingMenu);
         m_pPlacingMenu->clear();
         m_pPlacingMenu->insertItem(tr("Ca&scade windows"), m_pMdi,SLOT(cascadeWindows()));
         m_pPlacingMenu->insertItem(tr("Cascade &maximized"), m_pMdi,SLOT(cascadeMaximized()));
         m_pPlacingMenu->insertItem(tr("Expand &vertical"), m_pMdi,SLOT(expandVertical()));
         m_pPlacingMenu->insertItem(tr("Expand &horizontal"), m_pMdi,SLOT(expandHorizontal()));
         m_pPlacingMenu->insertItem(tr("A&nodine's tile"), m_pMdi,SLOT(tileAnodine()));
         m_pPlacingMenu->insertItem(tr("&Pragma's tile"), m_pMdi,SLOT(tilePragma()));
         m_pPlacingMenu->insertItem(tr("Tile v&ertically"), m_pMdi,SLOT(tileVertically()));
      m_pWindowMenu->insertSeparator();
      m_pWindowMenu->insertItem(tr("&Dock/Undock..."), m_pDockMenu);
         m_pDockMenu->clear();
      m_pWindowMenu->insertSeparator();
   }
   int entryCount = m_pWindowMenu->count();

   // for all child frame windows: give an ID to every window and connect them in the end with windowMenuItemActivated()
   int i=100;
   QextMdiChildView* pView = 0L;
   QListIterator<QextMdiChildView> it(*m_pWinList);
   for( ; it.current(); ++it) {

      pView = it.current();
      if( pView->isToolView())
         continue;

      QString item;
      // set titles of minimized windows in brackets
      if( pView->isMinimized()) {
         item += "(";
         item += pView->caption();
         item += ")";
      }
      else {
         item += " ";
         item += pView->caption();
       }

      // insert the window entry sorted in alphabetical order
      unsigned int indx;
      unsigned int windowItemCount = m_pWindowMenu->count() - entryCount;
      bool inserted = FALSE;
      QString tmpString;
      for (indx = 0; indx <= windowItemCount; indx++) {
         tmpString = m_pWindowMenu->text( m_pWindowMenu->idAt( indx+entryCount));
         if (tmpString.right( tmpString.length()-2) > item.right( item.length()-2)) {
            m_pWindowMenu->insertItem( item, pView, SLOT(slot_clickedInWindowMenu()), 0, -1, indx+entryCount);
            if (pView == m_pCurrentWindow)
               m_pWindowMenu->setItemChecked( m_pWindowMenu->idAt( indx+entryCount), TRUE);
            pView->setWindowMenuID( i);
            if (!bTabPageMode) {
               m_pDockMenu->insertItem( item, pView, SLOT(slot_clickedInDockMenu()), 0, -1, indx);
               if (pView->isAttached())
                  m_pDockMenu->setItemChecked( m_pDockMenu->idAt( indx), TRUE);
            }
            inserted = TRUE;
            indx = windowItemCount+1;  // break the loop
         }
      }
      if (!inserted) {  // append it
         m_pWindowMenu->insertItem( item, pView, SLOT(slot_clickedInWindowMenu()), 0, -1, windowItemCount+entryCount);
         if (pView == m_pCurrentWindow)
            m_pWindowMenu->setItemChecked( m_pWindowMenu->idAt(windowItemCount+entryCount), TRUE);
         pView->setWindowMenuID( i);
         if (!bTabPageMode) {
            m_pDockMenu->insertItem( item, pView, SLOT(slot_clickedInDockMenu()), 0, -1, windowItemCount);
            if (pView->isAttached())
               m_pDockMenu->setItemChecked( m_pDockMenu->idAt(windowItemCount), TRUE);
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
   if (m_mdiMode != QextMdi::TabPageMode)
      if ((pView == m_pMdi->topChild()->m_pClient) && pView->isAttached()) return;
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
   m_pWindowMenu->popup( p);
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
      m_frameDecoration = QextMdi::KDE2Look;
      break;
   case 3:
      m_frameDecoration = QextMdi::KDE2LaptopLook;
      break;
   default:
      qDebug("unknown MDI decoration");
      break;
   }
   setMenuForSDIModeSysButtons( m_pMainMenuBar);
   QListIterator<QextMdiChildView> it( *m_pWinList);
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

#ifndef NO_INCLUDE_MOCFILES
#include "qextmdimainfrm.moc"
#endif
