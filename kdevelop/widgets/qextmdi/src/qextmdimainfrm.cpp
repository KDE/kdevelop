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
#include "kde2_closebutton_menu.xpm"

#ifdef _OS_WIN32_
QextMdi::QextMdiFrameDecor QextMdiMainFrm::m_frameDecoration = QextMdi::Win95Look;
#else
#ifdef NO_KDE2
QextMdi::QextMdiFrameDecor QextMdiMainFrm::m_frameDecoration = QextMdi::KDE1Look;
#else
QextMdi::QextMdiFrameDecor QextMdiMainFrm::m_frameDecoration = QextMdi::KDE2Look;
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
   ,m_bTopLevelMode(FALSE)
   ,m_bMaximizedChildFrmMode(FALSE)
   ,m_oldMainFrmHeight(0)
   ,m_oldMainFrmMinHeight(0)
   ,m_oldMainFrmMaxHeight(0)
{
   // Create the local list of windows
   m_pWinList = new QList<QextMdiChildView>;
   m_pWinList->setAutoDelete(FALSE);
   // This seems to be needed (re-check it after Qt2.0 comed out)
   setFocusPolicy(ClickFocus);

   // And start creating self
   createMdiManager();
   createTaskBar();

   // Apply options for the MDI manager
   applyOptions();

   m_pTaskBarPopup=new QPopupMenu( this, "taskbar_popup_menu");
   m_pWindowPopup=new QPopupMenu( this, "window_popup_menu");

   m_pWindowMenu = new QPopupMenu( this, "window_menu");
   m_pWindowMenu->setCheckable( TRUE);
   QObject::connect( m_pWindowMenu, SIGNAL(aboutToShow()), this, SLOT(fillWindowMenu()) );

   m_pDockMenu = new QPopupMenu( this, "dock_menu");
   m_pDockMenu->setCheckable( TRUE);

   m_pPlacingMenu = new QPopupMenu( this, "placing_menu");
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
   QObject::connect( m_pMdi, SIGNAL(nowMaximized()), this, SLOT(setMaximizeModeOn()) );
   QObject::connect( m_pMdi, SIGNAL(noLongerMaximized(QextMdiChildFrm*)), this, SLOT(setMaximizeModeOff(QextMdiChildFrm*)) );
   QObject::connect( m_pMdi, SIGNAL(sysButtonConnectionsMustChange(QextMdiChildFrm*,QextMdiChildFrm*)), this, SLOT(updateSysButtonConnections(QextMdiChildFrm*,QextMdiChildFrm*)) );
   QObject::connect( m_pMdi, SIGNAL(popupWindowMenu(QPoint)), this, SLOT(popupWindowMenu(QPoint)) );
   QObject::connect( m_pMdi, SIGNAL(lastChildFrmClosed()), this, SIGNAL(lastChildFrmClosed()) );
}

//============ createTaskBar ==============//
void QextMdiMainFrm::createTaskBar()
{
   m_pTaskBar = new QextMdiTaskBar(this,QMainWindow::Bottom);
   m_pTaskBar->show();
   m_pTaskBar->installEventFilter( this);
}

void QextMdiMainFrm::slot_toggleTaskBar()
{
   if(m_pTaskBar->isVisible()){
      m_pTaskBar->hide();
   } else {
      m_pTaskBar->show();
   }
}

////============ closeEvent ============//
//void QextMdiMainFrm::closeEvent(QCloseEvent *e)
//{
//   e->accept();
//   delete this;
//}
//
void QextMdiMainFrm::resizeEvent(QResizeEvent *e)
{
   if( m_bTopLevelMode && !parentWidget())
      if( e->oldSize().height() != e->size().height()) {
         return;
      }
   KParts::DockMainWindow::resizeEvent(e);
}

//================ setMinimumSize ===============//

void QextMdiMainFrm::setMinimumSize( int minw, int minh)
{
   if( m_bTopLevelMode && !parentWidget())
      return;
   KParts::DockMainWindow::setMinimumSize( minw, minh);
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

   QObject::connect( pWnd, SIGNAL(attachWindow(QextMdiChildView*,bool)), this, SLOT(attachWindow(QextMdiChildView*,bool)) );
   QObject::connect( pWnd, SIGNAL(detachWindow(QextMdiChildView*,bool)), this, SLOT(detachWindow(QextMdiChildView*,bool)) );
   QObject::connect( pWnd, SIGNAL(focusInEventOccurs(QextMdiChildView*)), this, SLOT(activateView(QextMdiChildView*)) );
   QObject::connect( pWnd, SIGNAL(childWindowCloseRequest(QextMdiChildView*)), this, SLOT(childWindowCloseRequest(QextMdiChildView*)) );
   QObject::connect( pWnd, SIGNAL(clickedInWindowMenu(int)), this, SLOT(windowMenuItemActivated(int)) );
   QObject::connect( pWnd, SIGNAL(clickedInDockMenu(int)), this, SLOT(dockMenuItemActivated(int)) );

   m_pWinList->append(pWnd);
   QextMdiTaskBarButton* but = m_pTaskBar->addWinButton(pWnd);
   QObject::connect( pWnd, SIGNAL(tabCaptionChanged(const QString&)), but, SLOT(setNewText(const QString&)) );

   if( (flags & QextMdi::Detach) || m_bTopLevelMode) {
      detachWindow( pWnd, FALSE /*bShow*/ ); // FALSE to avoid flickering
   } else {
      attachWindow( pWnd, FALSE /*bShow*/);
   }

   if( flags & QextMdi::Maximize)
      pWnd->maximize();
   if( flags & QextMdi::Minimize)
      pWnd->minimize();
   if( !(flags & QextMdi::Hide)) {
      if( pWnd->isAttached()) {
         pWnd->mdiParent()->show();
      }
      else {
         pWnd->show();
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
void QextMdiMainFrm::addToolWindow( QextMdiChildView* pWnd)
{
   QRect r = pWnd->geometry();
   pWnd->reparent(this,WType_TopLevel | WStyle_StaysOnTop,r.topLeft(),pWnd->isVisible());
   QObject::connect( pWnd, SIGNAL(childWindowCloseRequest(QextMdiChildView*)), this, SLOT(childWindowCloseRequest(QextMdiChildView*)) );
   m_pWinList->append(pWnd);
   pWnd->m_bToolView = TRUE;
}

//============ attachWindow ============//
void QextMdiMainFrm::attachWindow(QextMdiChildView *pWnd, bool bShow)
{
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
//   if (pWnd->minimumHeight() == pWnd->maximumHeight())
//      lpC->setFixedHeight (lpC->height());
   pWnd->youAreAttached(lpC);
   if( m_bTopLevelMode && !parentWidget()) {
      setMinimumHeight( m_oldMainFrmMinHeight);
      setMaximumHeight( m_oldMainFrmMaxHeight);
      resize( width(), m_oldMainFrmHeight);
      m_oldMainFrmHeight = 0;
      m_bTopLevelMode = FALSE;
      qDebug("TopLevelMode off");
      emit leavedTopLevelMode();
   }

   // this is done in activateView
   //  m_pTaskBar->setActiveButton(pWnd);

   // position and really attach to MDI
   // this should add all the frame stuff but nothing more
   // remove the bShow from here
   m_pMdi->manageChild(lpC,FALSE,bCascade);
   if (bShow) {
      lpC->show();
   }

   // now you set the attribute like position and cascadin/maximize etc...
   // arrangeWindow(pWnd);

   // this will show it...
//   if (bShow) {
//      activateView(pWnd);
//   }
   m_pCurrentWindow  = pWnd;  // required for checking the active item
   QFocusEvent *fe = new QFocusEvent(QEvent::FocusIn);
   QApplication::sendEvent( pWnd, fe);
}

//============= detachWindow ==============//
void QextMdiMainFrm::detachWindow(QextMdiChildView *pWnd, bool bShow)
{

   // ok this part should be fine
   // NOPE! This is not to be set if bshow is FALSE: leave this to the activateView
   //  m_pCurrentWindow  = pWnd;

   pWnd->youAreDetached();

   // see above
   //  m_pTaskBar->setActiveButton(pWnd);

   // this is only if it was attached and you want to detach it
   if(pWnd->parent() != NULL ) {

      QextMdiChildFrm *lpC=pWnd->mdiParent();

      // this one is taking care of te
      lpC->unsetClient( m_undockPositioningOffset);

      m_pMdi->destroyChildButNotItsView(lpC,FALSE); //Do not focus the new top child , we loose focus...
   }
   else {
      if( pWnd->geometry() == QRect(0,0,1,1)) {
         pWnd->setGeometry( QRect( m_pMdi->getCascadePoint(m_pWinList->count()-1), defaultChildFrmSize()));
      }
   }

   // there should be an equivalent of manageChild... here
   //

   //  arrangeWindow(pWnd);

   // this will show it...
   if (bShow) {
      activateView(pWnd);
   }
}

//============== removeWindowFromMdi ==============//
void QextMdiMainFrm::removeWindowFromMdi(QextMdiChildView *pWnd)
{
   QObject::disconnect( pWnd, SIGNAL(attachWindow(QextMdiChildView*,bool)), this, SLOT(attachWindow(QextMdiChildView*,bool)) );
   QObject::disconnect( pWnd, SIGNAL(detachWindow(QextMdiChildView*,bool)), this, SLOT(detachWindow(QextMdiChildView*,bool)) );
   QObject::disconnect( pWnd, SIGNAL(focusInEventOccurs(QextMdiChildView*)), this, SLOT(activateView(QextMdiChildView*)) );
   QObject::disconnect( pWnd, SIGNAL(childWindowCloseRequest(QextMdiChildView*)), this, SLOT(childWindowCloseRequest(QextMdiChildView*)) );
   QObject::disconnect( pWnd, SIGNAL(clickedInWindowMenu(int)), this, SLOT(windowMenuItemActivated(int)) );
   QObject::disconnect( pWnd, SIGNAL(clickedInDockMenu(int)), this, SLOT(dockMenuItemActivated(int)) );

   //Closes a child window. sends no close event : simply deletes it
   m_pWinList->removeRef(pWnd);

   QextMdiTaskBarButton* but = m_pTaskBar->getButton(pWnd);
   // changed signale (mmorin)
   if (but != 0L) {
      QObject::disconnect( pWnd, SIGNAL(tabCaptionChanged(const QString&)), but, SLOT(setNewText(const QString&)) );
   }
   m_pTaskBar->removeWinButton(pWnd);

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
   if( m_pWinList->count() == 0L)
      m_pCurrentWindow = 0L;

   m_pTaskBar->removeWinButton(pWnd, layoutTaskBar);

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
   m_pTaskBar->setActiveButton(pWnd);

   if(pWnd->isAttached()){
      if( !(pWnd->hasFocus()) ) {
         // this should go out of here
         // (mmorin)
         if( m_pMdi->topChild()->state() == QextMdiChildFrm::Maximized) {
            updateSysButtonConnections( m_pMdi->topChild(), pWnd->mdiParent());
         }
      }
   }
   else {
      // this was the one not too cool
      // see here you are not doing anything if it is maximize...
      if(!pWnd->hasFocus() || !pWnd->isActiveWindow()) {
         pWnd->show();
         pWnd->setActiveWindow();
         pWnd->raise();
      }
   }
   pWnd->setFocus();
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

//##################################################
// Internal handlers
//##################################################

void QextMdiMainFrm::focusInEvent(QFocusEvent *)
{
   //qDebug("QextMdiMainFrm::focusInEvent");
   m_pMdi->setFocus();
}

bool QextMdiMainFrm::event( QEvent* e)
{
   if( e->type() == QEvent::User) {
      QextMdiChildView* pWnd = (QextMdiChildView*)((QextMdiViewCloseEvent*)e)->data();
      if( pWnd != 0L)
         closeWindow( pWnd);
      return TRUE;
   }
   return KParts::DockMainWindow::event( e);
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
   if( m_pCurrentWindow != 0L)
      m_pCurrentWindow->close();
}

/**
 * undocks all view windows (unix-like)
 */
void QextMdiMainFrm::switchToToplevelMode()
{
   QListIterator<QextMdiChildView> it( *m_pWinList);
   for( ; it.current(); ++it) {
      QextMdiChildView* pView = it.current();
      if( pView->isToolView())
         continue;
      if( pView->isAttached()) {
         if( pView->isMaximized())
            pView->mdiParent()->setGeometry( 0, 0, m_pMdi->width(), m_pMdi->height());
         detachWindow( pView, TRUE);
      }
   }
   if(!parentWidget()) {
      m_oldMainFrmMinHeight = minimumHeight();
      m_oldMainFrmMaxHeight = maximumHeight();
      m_oldMainFrmHeight = height();
      if( m_pWinList->count())
         setFixedHeight( height() - m_pMdi->height());
      else { // consider space for the taskbar
         QApplication::sendPostedEvents();
         setFixedHeight( height() - m_pMdi->height() + 27);
      }
   }
   m_bTopLevelMode = TRUE;
   qDebug("ToplevelMode on");
}

/**
 * docks all view windows (Windows-like)
 */
void QextMdiMainFrm::switchToChildframeMode()
{
   QListIterator<QextMdiChildView> it( *m_pWinList);
   for( ; it.current(); ++it) {
      QextMdiChildView* pView = it.current();
      if( !pView->isToolView())
         if( !pView->isAttached())
            attachWindow( pView, TRUE);
   }
   if( m_bTopLevelMode && !parentWidget()) {
      setMinimumHeight( m_oldMainFrmMinHeight);
      setMaximumHeight( m_oldMainFrmMaxHeight);
      resize( width(), m_oldMainFrmHeight);
      m_oldMainFrmHeight = 0;
      m_bTopLevelMode = FALSE;
      qDebug("TopLevelMode off");
      emit leavedTopLevelMode();
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
   else {
      m_pUndockButtonPixmap = new QPixmap( kde2_undockbutton);
      m_pMinButtonPixmap = new QPixmap( kde2_minbutton);
      m_pRestoreButtonPixmap = new QPixmap( kde2_restorebutton);
      m_pCloseButtonPixmap = new QPixmap( kde2_closebutton);
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
   else
      h = 14;
   y = m_pMainMenuBar->height()/2 - h/2;

   if (frameDecorOfAttachedViews() == QextMdi::KDE2Look) {
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
void QextMdiMainFrm::setMaximizeModeOn()
{
   QextMdiChildFrm* pCurrentChild = m_pMdi->topChild();
   if( !pCurrentChild)
      return;

   m_bMaximizedChildFrmMode = TRUE;
   qDebug("MaximizeMode on");

   // if there is no menubar given, those system buttons aren't possible
   if( m_pMainMenuBar == 0L)
      return;
      
   QObject::connect( m_pUndock, SIGNAL(clicked()), pCurrentChild, SLOT(undockPressed()) );
   m_pUndock->show();
   QObject::connect( m_pMinimize, SIGNAL(clicked()), pCurrentChild, SLOT(minimizePressed()) );
   m_pMinimize->show();
   QObject::connect( m_pRestore, SIGNAL(clicked()), pCurrentChild, SLOT(maximizePressed()) );
   m_pRestore->show();

   if (frameDecorOfAttachedViews() == QextMdi::KDE2Look) {
      m_pMainMenuBar->insertItem( QPixmap(kde2_closebutton_menu), m_pMdi->topChild(), SLOT(closePressed()), 0, -1, 0);
   }
   else {
      m_pMainMenuBar->insertItem( *pCurrentChild->icon(), pCurrentChild->systemMenu(), -1, 0);
      QObject::connect( m_pClose, SIGNAL(clicked()), pCurrentChild, SLOT(closePressed()) );
      m_pClose->show();
   }
}

/** turns the system buttons for maximize mode (SDI mode) off, and disconnects them */
void QextMdiMainFrm::setMaximizeModeOff(QextMdiChildFrm* oldChild)
{
   if( !oldChild)
      return;

   m_bMaximizedChildFrmMode = TRUE;
   qDebug("MaximizeMode off");

   // if there is no menubar given, those system buttons aren't possible
   if( m_pMainMenuBar == 0L)
      return;
      
   m_pMainMenuBar->removeItem( m_pMainMenuBar->idAt(0));

   QObject::disconnect( m_pUndock, SIGNAL(clicked()), oldChild, SLOT(undockPressed()) );
   m_pUndock->hide();
   QObject::disconnect( m_pMinimize, SIGNAL(clicked()), oldChild, SLOT(minimizePressed()) );
   m_pMinimize->hide();
   QObject::disconnect( m_pRestore, SIGNAL(clicked()), oldChild, SLOT(maximizePressed()) );
   m_pRestore->hide();
   QObject::disconnect( m_pClose, SIGNAL(clicked()), oldChild, SLOT(closePressed()) );
   m_pClose->hide();
}

/** reconnects the system buttons form maximize mode (SDI mode) with the new child frame */
void QextMdiMainFrm::updateSysButtonConnections( QextMdiChildFrm* oldChild, QextMdiChildFrm* newChild)
{
   // if there is no menubar given, those system buttons aren't possible
   if( m_pMainMenuBar == 0L)
      return;
      
   QObject::disconnect( m_pUndock, SIGNAL(clicked()), oldChild, SLOT(undockPressed()) );
   QObject::disconnect( m_pMinimize, SIGNAL(clicked()), oldChild, SLOT(minimizePressed()) );
   QObject::disconnect( m_pRestore, SIGNAL(clicked()), oldChild, SLOT(maximizePressed()) );
   QObject::disconnect( m_pClose, SIGNAL(clicked()), oldChild, SLOT(closePressed()) );

   QObject::connect( m_pUndock, SIGNAL(clicked()), newChild, SLOT(undockPressed()) );
   QObject::connect( m_pMinimize, SIGNAL(clicked()), newChild, SLOT(minimizePressed()) );
   QObject::connect( m_pRestore, SIGNAL(clicked()), newChild, SLOT(maximizePressed()) );
   QObject::connect( m_pClose, SIGNAL(clicked()), newChild, SLOT(closePressed()) );
}

/** Shows the view taskbar. This should be connected with your "View" menu. */
void QextMdiMainFrm::showViewTaskBar()
{
   m_pTaskBar->show();
}

/** Hides the view taskbar. This should be connected with your "View" menu. */
void QextMdiMainFrm::hideViewTaskBar()
{
   m_pTaskBar->hide();
}

//=============== fillWindowMenu ===============//
void QextMdiMainFrm::fillWindowMenu()
{
   m_pWindowMenu->clear();
   m_pWindowMenu->insertItem(tr("&Close"), this, SLOT(closeActiveView()));
   m_pWindowMenu->insertItem(tr("Close &All"), this, SLOT(closeAllViews()));
   m_pWindowMenu->insertItem(tr("&Iconify All"), this, SLOT(iconifyAllViews()));
   m_pWindowMenu->insertSeparator();
   m_pWindowMenu->insertItem(tr("&Placing..."), m_pPlacingMenu);
   m_pWindowMenu->insertSeparator();
   m_pWindowMenu->insertItem(tr("&Dock/Undock..."), m_pDockMenu);
   m_pWindowMenu->insertSeparator();

   m_pPlacingMenu->clear();
   m_pPlacingMenu->insertItem(tr("Ca&scade windows"), m_pMdi,SLOT(cascadeWindows()));
   m_pPlacingMenu->insertItem(tr("Cascade &maximized"), m_pMdi,SLOT(cascadeMaximized()));
   m_pPlacingMenu->insertItem(tr("Expand &vertical"), m_pMdi,SLOT(expandVertical()));
   m_pPlacingMenu->insertItem(tr("Expand &horizontal"), m_pMdi,SLOT(expandHorizontal()));
   m_pPlacingMenu->insertItem(tr("A&nodine's tile"), m_pMdi,SLOT(tileAnodine()));
   m_pPlacingMenu->insertItem(tr("&Pragma's tile"), m_pMdi,SLOT(tilePragma()));
   m_pPlacingMenu->insertItem(tr("Tile v&ertically"), m_pMdi,SLOT(tileVertically()));

   m_pDockMenu->clear();
   m_pDockMenu->insertItem(tr("&Toplevel mode"), this, SLOT(switchToToplevelMode()));
   m_pDockMenu->insertItem(tr("C&hildframe mode"), this, SLOT(switchToChildframeMode()));
   m_pDockMenu->insertSeparator();

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
      unsigned int windowItemCount = m_pWindowMenu->count() - 8;
      bool inserted = FALSE;
      QString tmpString;
      for( indx = 0; indx <= windowItemCount; indx++) {
         tmpString = m_pWindowMenu->text( m_pWindowMenu->idAt( indx+8));
         if( tmpString.right( tmpString.length()-2) > item.right( item.length()-2)) {
            m_pWindowMenu->insertItem( item, pView, SLOT( slot_clickedInWindowMenu()), 0, -1, indx+8);
            m_pDockMenu->insertItem( item, pView, SLOT( slot_clickedInDockMenu()), 0, -1, indx+3);
            if( pView == m_pCurrentWindow)
               m_pWindowMenu->setItemChecked( m_pWindowMenu->idAt( indx+8), TRUE);
            pView->setWindowMenuID( i);
            if( pView->isAttached())
               m_pDockMenu->setItemChecked( m_pDockMenu->idAt( indx+3), TRUE);
            inserted = TRUE;
            indx = windowItemCount+1;  // break the loop
         }
      }
      if( !inserted) {  // append it
         m_pWindowMenu->insertItem( item, pView, SLOT( slot_clickedInWindowMenu()), 0, -1, windowItemCount+8);
         if( pView == m_pCurrentWindow)
            m_pWindowMenu->setItemChecked( m_pWindowMenu->idAt( windowItemCount+8), TRUE);
         pView->setWindowMenuID( i);
         m_pDockMenu->insertItem( item, pView, SLOT( slot_clickedInDockMenu()), 0, -1, windowItemCount+3);
         if( pView->isAttached())
            m_pDockMenu->setItemChecked( m_pDockMenu->idAt( windowItemCount+3), TRUE);
      }
      i++;
   }
}

//================ windowMenuItemActivated ===============//

void QextMdiMainFrm::windowMenuItemActivated(int id)
{
   if( id < 100) return;
   id -= 100;
   QextMdiChildView *pView = m_pWinList->at( id);
   if( !pView) return;
   if( pView->isMinimized()) pView->minimize();
   if( (pView == m_pMdi->topChild()->m_pClient) && pView->isAttached()) return;
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

#ifndef NO_INCLUDE_MOCFILES
#include "qextmdimainfrm.moc"
#endif
