//----------------------------------------------------------------------------
//    filename             : qextmdichildfrm.cpp
//----------------------------------------------------------------------------
//    Project              : Qt MDI extension
//
//    begin                : 07/1999       by Szymon Stefanek as part of kvirc
//                                         (an IRC application)
//    changes              : 09/1999       by Falk Brettschneider to create an
//                           - 06/2000     stand-alone Qt extension set of
//                                         classes and a Qt-based library
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

#include <qpainter.h>
#include <qapplication.h>
#include <qcursor.h>
#include <qobjectlist.h>

#include <qnamespace.h>

#include "qextmdidefines.h"
#include "qextmdichildfrmcaption.h"
#include "qextmdichildarea.h"
#include "qextmdichildfrm.h"

//////////////////////////////////////////////////////////////////////////////
// QextMdiChildFrm
//////////////////////////////////////////////////////////////////////////////

#define QEXTMDI_NORESIZE 0
#define QEXTMDI_RESIZE_TOP 1
#define QEXTMDI_RESIZE_LEFT 2
#define QEXTMDI_RESIZE_RIGHT 4
#define QEXTMDI_RESIZE_BOTTOM 8
#define QEXTMDI_RESIZE_TOPLEFT (1|2)
#define QEXTMDI_RESIZE_TOPRIGHT (1|4)
#define QEXTMDI_RESIZE_BOTTOMLEFT (8|2)
#define QEXTMDI_RESIZE_BOTTOMRIGHT (8|4)

#include "filenew.xpm"
#ifdef _OS_WIN32_
#  include "win_closebutton.xpm"
#  include "win_minbutton.xpm"
#  include "win_maxbutton.xpm"
#  include "win_restorebutton.xpm"
#  include "win_undockbutton.xpm"

#else // in case of UNIX: KDE look
#  include "kde_closebutton.xpm"
#  include "kde_minbutton.xpm"
#  include "kde_maxbutton.xpm"
#  include "kde_restorebutton.xpm"
#  include "kde_undockbutton.xpm"
#endif

QextMdiWin32IconButton::QextMdiWin32IconButton( QWidget* parent, const char* name)
  : QLabel( parent, name)
{
}

//============ mousePressEvent ============//

void QextMdiWin32IconButton::mousePressEvent( QMouseEvent*)
{
   emit pressed();
}

//============ QextMdiChildFrm ============//

QextMdiChildFrm::QextMdiChildFrm(QextMdiChildArea *parent)
 : QFrame(parent, "qextmdi_childfrm")
  ,m_pClient(0)
  ,m_resizeMode(FALSE)
  ,m_windowMenuID(0)
{
   m_pCaption  = new QextMdiChildFrmCaption(this);
   m_pManager  = parent;

#ifdef _OS_WIN32_
   m_pIcon     = new QextMdiWin32IconButton(m_pCaption, "qextmdi_iconbutton_icon");
   m_pMinimize = new QPushButton(m_pCaption, "qextmdi_pushbutton_min");
   m_pMaximize = new QPushButton(m_pCaption, "qextmdi_pushbutton_max");
   m_pClose    = new QPushButton(m_pCaption, "qextmdi_pushbutton_close");
   m_pUndock   = new QPushButton(m_pCaption, "qextmdi_pushbutton_undock");
#else // in case of UNIX: KDE look
   m_pIcon     = new QToolButton(m_pCaption, "qextmdi_toolbutton_icon");
   m_pMinimize = new QToolButton(m_pCaption, "qextmdi_toolbutton_min");
   m_pMaximize = new QToolButton(m_pCaption, "qextmdi_toolbutton_max");
   m_pClose    = new QToolButton(m_pCaption, "qextmdi_toolbutton_close");
   m_pUndock   = new QToolButton(m_pCaption, "qextmdi_toolbutton_undock");
#endif

#if QT_VERSION > 209
#ifndef _OS_WIN32_
   m_pIcon->setAutoRaise(TRUE);
#endif
#endif
#if QT_VERSION > 209
#ifndef _OS_WIN32_
   m_pMinimize->setAutoRaise(TRUE);
   m_pMaximize->setAutoRaise(TRUE);
   m_pClose->setAutoRaise(TRUE);
   m_pUndock->setAutoRaise(TRUE);
#endif
#endif

   QObject::connect(m_pIcon,SIGNAL(pressed()),this,SLOT(showSystemMenu()));
   QObject::connect(m_pMinimize,SIGNAL(clicked()),this,SLOT(minimizePressed()));
   QObject::connect(m_pMaximize,SIGNAL(clicked()),this,SLOT(maximizePressed()));
   QObject::connect(m_pClose,SIGNAL(clicked()),this,SLOT(closePressed()));
   QObject::connect(m_pUndock,SIGNAL(clicked()),this,SLOT(undockPressed()));

   m_pIconButtonPixmap = new QPixmap( filenew);
#ifdef _OS_WIN32_
   m_pMinButtonPixmap = new QPixmap( win_minbutton);
   m_pMaxButtonPixmap = new QPixmap( win_maxbutton);
   m_pRestoreButtonPixmap = new QPixmap( win_restorebutton);
   m_pCloseButtonPixmap = new QPixmap( win_closebutton);
   m_pUndockButtonPixmap = new QPixmap( win_undockbutton);
#else // in case of UNIX: KDE look
   m_pMinButtonPixmap = new QPixmap( kde_minbutton);
   m_pMaxButtonPixmap = new QPixmap( kde_maxbutton);
   m_pRestoreButtonPixmap = new QPixmap( kde_restorebutton);
   m_pCloseButtonPixmap = new QPixmap( kde_closebutton);
   m_pUndockButtonPixmap = new QPixmap( kde_undockbutton);
#endif

   m_pIcon->setPixmap( *m_pIconButtonPixmap);
   m_pIcon->setFocusPolicy(NoFocus);
   m_pClose->setPixmap( *m_pCloseButtonPixmap);
   m_pClose->setFocusPolicy(NoFocus);
   m_pMinimize->setPixmap( *m_pMinButtonPixmap);
   m_pMinimize->setFocusPolicy(NoFocus);
   m_pMaximize->setPixmap( *m_pMaxButtonPixmap);
   m_pMaximize->setFocusPolicy(NoFocus);
   m_pUndock->setPixmap( *m_pUndockButtonPixmap);
   m_pUndock->setFocusPolicy(NoFocus);

   setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   setFocusPolicy(NoFocus);

   m_state=Normal;
   
   m_iResizeCorner=QEXTMDI_NORESIZE;
   m_iLastCursorCorner=QEXTMDI_NORESIZE;
   setMouseTracking(TRUE);

   setMinimumSize( QSize( QEXTMDI_MDI_CHILDFRM_MIN_WIDTH, m_pCaption->heightHint()));

   m_pSystemMenu = new QPopupMenu();
}

//============ ~QextMdiChildFrm ============//

QextMdiChildFrm::~QextMdiChildFrm()
{
   delete m_pMinButtonPixmap;
   delete m_pMaxButtonPixmap;
   delete m_pRestoreButtonPixmap;
   delete m_pCloseButtonPixmap;
   delete m_pUndockButtonPixmap;
}

//============ mousePressEvent =============//
void QextMdiChildFrm::mousePressEvent(QMouseEvent *e)
{
   if( m_resizeMode) {
      if(QApplication::overrideCursor())QApplication::restoreOverrideCursor();
      m_resizeMode = FALSE;
      releaseMouse();
   }

   m_pCaption->setActive(TRUE);
   m_pManager->setTopChild(this,FALSE);

   m_iResizeCorner=getResizeCorner(e->pos().x(),e->pos().y());
   if(m_iResizeCorner != QEXTMDI_NORESIZE) {
      m_resizeMode = TRUE;
   }
}

//============ mouseReleaseEvent ==============//

void QextMdiChildFrm::mouseReleaseEvent(QMouseEvent *)
{
   if(QApplication::overrideCursor())QApplication::restoreOverrideCursor();
   m_resizeMode = FALSE;
}

//============= setResizeCursor ===============//

void QextMdiChildFrm::setResizeCursor(int resizeCorner)
{
   if(resizeCorner == m_iLastCursorCorner)return; //Don't do it twice
   m_iLastCursorCorner = resizeCorner;
   switch (resizeCorner) {
      case QEXTMDI_NORESIZE:
         if(QApplication::overrideCursor())QApplication::restoreOverrideCursor();
         break;
      case QEXTMDI_RESIZE_LEFT:
      case QEXTMDI_RESIZE_RIGHT:
         QApplication::setOverrideCursor(Qt::sizeHorCursor,TRUE);
         break;
      case QEXTMDI_RESIZE_TOP:
      case QEXTMDI_RESIZE_BOTTOM:
         QApplication::setOverrideCursor(Qt::sizeVerCursor,TRUE);
         break;
      case QEXTMDI_RESIZE_TOPLEFT:
      case QEXTMDI_RESIZE_BOTTOMRIGHT:
         QApplication::setOverrideCursor(Qt::sizeFDiagCursor,TRUE);
         break;
      case QEXTMDI_RESIZE_BOTTOMLEFT:
      case QEXTMDI_RESIZE_TOPRIGHT:
         QApplication::setOverrideCursor(Qt::sizeBDiagCursor,TRUE);
         break;
   }
}

//============= mouseMoveEvent ===============//

void QextMdiChildFrm::mouseMoveEvent(QMouseEvent *e)
{
   if(m_state != Normal) return;
   if(m_pClient->minimumSize() == m_pClient->maximumSize()) return;

   if(m_resizeMode) {
      if( !(e->state() & RightButton) && !(e->state() & MidButton)) {
         // same as: if no button or left button pressed
         QPoint p = parentWidget()->mapFromGlobal( e->globalPos() );
         resizeWindow(m_iResizeCorner, p.x(), p.y());
      }
      else
         m_resizeMode = FALSE;
   }
   else {
      m_iResizeCorner = getResizeCorner(e->pos().x(), e->pos().y());
      setResizeCursor( m_iResizeCorner);
   }
}

//============= moveEvent ===============//

void QextMdiChildFrm::moveEvent(QMoveEvent* me)
{
   // give its child view the chance to notify a childframe move
   QextMdiChildFrmMovedEvent cfme( me);
   if( m_pClient != 0L) {
      QApplication::sendEvent( m_pClient, &cfme);
   }
}

//=============== leaveEvent ===============//

void QextMdiChildFrm::leaveEvent(QEvent *)
{
   if(!m_resizeMode) {
      m_iResizeCorner=QEXTMDI_NORESIZE;
      m_iLastCursorCorner=QEXTMDI_NORESIZE;
      if(QApplication::overrideCursor())QApplication::restoreOverrideCursor();
   }
}

void QextMdiChildFrm::resizeWindow(int resizeCorner, int xPos, int yPos)
{
   QRect resizeRect(x(),y(),width(),height());

   // Calculate the minimum width & height
   int minWidth=0;
   int minHeight=0;
   // it could be the client forces the childframe to enlarge its minimum size
   if(m_pClient){
      minWidth  = m_pClient->minimumSize().width() + QEXTMDI_MDI_CHILDFRM_DOUBLE_BORDER;
      minHeight = m_pClient->minimumSize().height()+ QEXTMDI_MDI_CHILDFRM_DOUBLE_BORDER +
               m_pCaption->heightHint() + QEXTMDI_MDI_CHILDFRM_SEPARATOR;
   }
   if(minWidth<minimumWidth())minWidth=minimumWidth();
   if(minHeight<minimumHeight())minHeight=minimumHeight();

   QPoint mousePos( xPos, yPos);
   
   switch (resizeCorner){
   case QEXTMDI_RESIZE_LEFT:
      resizeRect.setLeft(mousePos.x());
      if(resizeRect.width() < minWidth)resizeRect.setLeft(resizeRect.right() - minWidth);
      break;
   case QEXTMDI_RESIZE_RIGHT:
      resizeRect.setRight(mousePos.x());
      if(resizeRect.width() < minWidth)resizeRect.setRight(resizeRect.left() + minWidth);
      break;
   case QEXTMDI_RESIZE_TOP:
      resizeRect.setTop(mousePos.y());
      if(resizeRect.height() < minHeight)resizeRect.setTop(resizeRect.bottom() - minHeight);
      break;
   case QEXTMDI_RESIZE_BOTTOM:
      resizeRect.setBottom(mousePos.y());
      if(resizeRect.height() < minHeight)resizeRect.setBottom(resizeRect.top() + minHeight);
      break;
   case QEXTMDI_RESIZE_BOTTOMRIGHT:
      resizeRect.setBottom(mousePos.y());
      if(resizeRect.height() < minHeight)resizeRect.setBottom(resizeRect.top() + minHeight);
      resizeRect.setRight(mousePos.x());
      if(resizeRect.width() < minWidth)resizeRect.setRight(resizeRect.left() + minWidth);
      break;
   case QEXTMDI_RESIZE_TOPRIGHT:
      resizeRect.setTop(mousePos.y());
      if(resizeRect.height() < minHeight)resizeRect.setTop(resizeRect.bottom() - minHeight);
      resizeRect.setRight(mousePos.x());
      if(resizeRect.width() < minWidth)resizeRect.setRight(resizeRect.left() + minWidth);
      break;
   case QEXTMDI_RESIZE_BOTTOMLEFT:
      resizeRect.setBottom(mousePos.y());
      if(resizeRect.height() < minHeight)resizeRect.setBottom(resizeRect.top() + minHeight);
      resizeRect.setLeft(mousePos.x());         
      if(resizeRect.width() < minWidth)resizeRect.setLeft(resizeRect.right() - minWidth);
      break;
   case QEXTMDI_RESIZE_TOPLEFT:
      resizeRect.setTop(mousePos.y());
      if(resizeRect.height() < minHeight)resizeRect.setTop(resizeRect.bottom() - minHeight);
      resizeRect.setLeft(mousePos.x());
      if(resizeRect.width() < minWidth)resizeRect.setLeft(resizeRect.right() - minWidth);
      break;
   }

   setGeometry( resizeRect);

   if(m_state==Maximized){
      m_state=Normal;
      m_pMaximize->setPixmap( *m_pMaxButtonPixmap);
   }
}

//================= getResizeCorner =============//

int QextMdiChildFrm::getResizeCorner(int ax,int ay)
{
   int ret = QEXTMDI_NORESIZE;
   if(m_pClient->minimumWidth() != m_pClient->maximumWidth()) {
   if((ax>0)&&(ax<(QEXTMDI_MDI_CHILDFRM_BORDER+2))) ret |= QEXTMDI_RESIZE_LEFT;
     if((ax<width())&&(ax>(width()-(QEXTMDI_MDI_CHILDFRM_BORDER+2)))) ret |= QEXTMDI_RESIZE_RIGHT;
   }
  if(m_pClient->minimumHeight() != m_pClient->maximumHeight()) {
   if((ay>0)&&(ay<(QEXTMDI_MDI_CHILDFRM_BORDER+2))) ret |= QEXTMDI_RESIZE_TOP;
     if((ay<(height()))&&(ay>(height()-(QEXTMDI_MDI_CHILDFRM_BORDER+2)))) ret |= QEXTMDI_RESIZE_BOTTOM;
   }
   return ret;
}

//============= maximizePressed ============//

void QextMdiChildFrm::maximizePressed()
{
   switch(m_state){
      case Maximized:
         emit m_pManager->noLongerMaximized(this);
         setState(Normal);
         break;
      case Normal:
         setState(Maximized);
         emit m_pManager->nowMaximized();
         break;
      case Minimized:
         setState(Maximized);
         emit m_pManager->nowMaximized();
         break;
   }
}

void QextMdiChildFrm::restorePressed()
{
   if( m_state == Normal)
      return;
   if( m_state == Maximized)
      emit m_pManager->noLongerMaximized(this);
   setState(Normal);
}

//============= minimizePressed ============//

void QextMdiChildFrm::minimizePressed()
{
   switch(m_state){
      case Minimized: setState(Normal);    break;
      case Normal:    setState(Minimized); break;
      case Maximized:
         emit m_pManager->noLongerMaximized(this);
         setState(Normal);
         setState(Minimized);
         break;
   }
}

//============= closePressed ============//

void QextMdiChildFrm::closePressed()
{
   if(m_pClient)
      m_pClient->close();
}

//============= undockPressed ============//

void QextMdiChildFrm::undockPressed()
{
   if(m_pClient) {
      if( m_state == Minimized)
         setState( Normal);
      m_pManager->undockWindow(m_pClient);
   }
}

//============ setState =================//

void QextMdiChildFrm::setState(MdiWindowState state, bool /*bAnimate*/)
{
   if(m_state==Normal){ //save the current rect
      m_restoredRect=QRect(x(),y(),width(),height());
   }
   //QRect begin(x(),y(),width(),height());
   //QRect end=begin;
   switch(state){
   case Normal:
      switch(m_state){
      case Maximized:
         m_pClient->m_stateChanged = true;
         m_state=state;
         //F.B. if(bAnimate)m_pManager->animate(begin,m_restoredRect);
         m_pClient->setMinimumSize(m_oldClientMinSize.width(),m_oldClientMinSize.height());
         m_pClient->setMaximumSize(m_oldClientMaxSize.width(),m_oldClientMaxSize.height());
         setMinimumWidth(QEXTMDI_MDI_CHILDFRM_MIN_WIDTH);
         m_pMaximize->setPixmap( *m_pMaxButtonPixmap);
         setGeometry(m_restoredRect);
         break;
      case Minimized:
         m_pClient->m_stateChanged = true;
         m_state=state;
         //begin=QRect(x()+width()/2,y()+height()/2,1,1);
         //if(bAnimate)m_pManager->animate(begin,end);
         m_pClient->show();
         m_pClient->setMinimumSize(m_oldClientMinSize.width(),m_oldClientMinSize.height());
         m_pClient->setMaximumSize(m_oldClientMaxSize.width(),m_oldClientMaxSize.height());
         setMinimumWidth(QEXTMDI_MDI_CHILDFRM_MIN_WIDTH);
         // reset to normal-captionbar
         m_pMinimize->setPixmap( *m_pMinButtonPixmap);
         m_pMaximize->setPixmap( *m_pMaxButtonPixmap);
         QObject::disconnect(m_pMinimize,SIGNAL(clicked()),this,SLOT(restorePressed()));
         QObject::connect(m_pMinimize,SIGNAL(clicked()),this,SLOT(minimizePressed()));
         setGeometry(m_restoredRect);
         break;
      case Normal:
          break;
      }
      break;
   case Maximized:
      //end=QRect(0,0,m_pManager->width(),m_pManager->height());
      switch(m_state){
      case Minimized:
         m_pClient->m_stateChanged = true;
         //begin=QRect(x()+width()/2,y()+height()/2,1,1);
         //if(bAnimate)m_pManager->animate(begin,end);
         m_pClient->show();
         m_pClient->setMinimumSize(m_oldClientMinSize.width(),m_oldClientMinSize.height());
         m_pClient->setMaximumSize(m_oldClientMaxSize.width(),m_oldClientMaxSize.height());
         setMaximumSize(QWIDGETSIZE_MAX,QWIDGETSIZE_MAX);
         // reset to maximize-captionbar
         m_pMaximize->setPixmap( *m_pRestoreButtonPixmap);
         m_pMinimize->setPixmap( *m_pMinButtonPixmap);
         QObject::disconnect(m_pMinimize,SIGNAL(clicked()),this,SLOT(restorePressed()));
         QObject::connect(m_pMinimize,SIGNAL(clicked()),this,SLOT(minimizePressed()));
         m_state=state;
         setGeometry(-m_pClient->x(), -m_pClient->y(),
                     m_pManager->width() + width() - m_pClient->width(),
                     m_pManager->height() + height() - m_pClient->height());
         raise();
         break;
      case Normal:
         m_pClient->m_stateChanged = true;
         m_state=state;
         m_oldClientMinSize = m_pClient->minimumSize();
         m_oldClientMaxSize = m_pClient->maximumSize();
         show();
         m_pClient->setMaximumSize(QWIDGETSIZE_MAX,QWIDGETSIZE_MAX);
         setMaximumSize(QWIDGETSIZE_MAX,QWIDGETSIZE_MAX);
         //if(bAnimate)m_pManager->animate(begin,end);
         m_pMaximize->setPixmap( *m_pRestoreButtonPixmap);
         setGeometry(-m_pClient->x(), -m_pClient->y(),
                         m_pManager->width() + width() - m_pClient->width(),
                     m_pManager->height() + height() - m_pClient->height());
         raise();
         break;
      case Maximized:
         break;
      }
   break;
   case Minimized:
      //end=QRect(x()+width()/2,y()+height()/2,1,1);
      switch(m_state){
      case Maximized:
         m_pClient->m_stateChanged = true;
         m_state=state;
         switchToMinimizeLayout();
         //if(bAnimate)m_pManager->animate(begin,end);
         m_pManager->childMinimized(this,TRUE);
         break;
      case Normal:
         m_pClient->m_stateChanged = true;
         m_state=state;
         m_oldClientMinSize = m_pClient->minimumSize();
         m_oldClientMaxSize = m_pClient->maximumSize();
         m_restoredRect = geometry();
         switchToMinimizeLayout();
         //if(bAnimate)m_pManager->animate(begin,end);
         m_pManager->childMinimized(this,FALSE);
         break;
      case Minimized:
         break;
      }
   break;
   }
}

//============== restoreGeometry ================//

QRect QextMdiChildFrm::restoreGeometry() const
{
   return m_restoredRect;
}

//============== setRestoreGeometry ================//

void QextMdiChildFrm::setRestoreGeometry(const QRect& newRestGeo)
{
   m_restoredRect = newRestGeo;
}

//============ setCaption ===============//

void QextMdiChildFrm::setCaption(const QString& text)
{
   m_pCaption->setCaption(text);
}

//============ enableClose ==============//

void QextMdiChildFrm::enableClose(bool bEnable)
{
   m_pClose->setEnabled(bEnable);
   m_pClose->repaint(FALSE);
}

//============ setIcon ==================//

void QextMdiChildFrm::setIcon(const QPixmap& pxm)
{
   *m_pIconButtonPixmap = pxm;
   m_pIcon->setPixmap( pxm);
}

//============ icon =================//

QPixmap* QextMdiChildFrm::icon()
{
   return m_pIconButtonPixmap;
}

//============ setClient ============//
void QextMdiChildFrm::setClient(QextMdiChildView *w)
{
   m_pClient=w;
   //resize to match the client
   int clientYPos=m_pCaption->heightHint()+QEXTMDI_MDI_CHILDFRM_SEPARATOR+QEXTMDI_MDI_CHILDFRM_BORDER;
  if(w->size().isEmpty() || (w->size() == QSize(1,1)))
   resize(m_pManager->m_defaultChildFrmSize.width()+QEXTMDI_MDI_CHILDFRM_DOUBLE_BORDER,m_pManager->m_defaultChildFrmSize.height()+QEXTMDI_MDI_CHILDFRM_BORDER+clientYPos);
  else
   resize(w->width()+QEXTMDI_MDI_CHILDFRM_DOUBLE_BORDER,w->height()+QEXTMDI_MDI_CHILDFRM_BORDER+clientYPos);

   // memorize the focuses in a dictionary because they will get lost during reparenting
   QDict<FocusPolicy>* pFocPolDict = new QDict<FocusPolicy>;
   pFocPolDict->setAutoDelete(TRUE);
   QObjectList *list = m_pClient->queryList( "QWidget" );
   QObjectListIt it( *list );          // iterate over the buttons
   QObject * obj;
   int i = 1;
   while ( (obj=it.current()) != 0 ) { // for each found object...
      ++it;
      QWidget* widg = (QWidget*)obj;
      if( widg->name(0) == 0) {
         QString tmpStr;
         tmpStr.setNum( i);
         tmpStr = "unnamed" + tmpStr;
         widg->setName( tmpStr);
         i++;
      }
      FocusPolicy* pFocPol = new FocusPolicy;
      *pFocPol = widg->focusPolicy();
      pFocPolDict->insert( widg->name(), pFocPol);
   }
   delete list;                        // delete the list, not the objects
   
   //Reparent if needed
   if(w->parent()!=this){
      //reparent to this widget , no flags , point , show it
      QPoint pnt2(QEXTMDI_MDI_CHILDFRM_BORDER,clientYPos);
      QSize mincs = w->minimumSize();
      QSize maxcs = w->maximumSize();
      w->setMinimumSize(0,0);
      w->setMaximumSize(QWIDGETSIZE_MAX,QWIDGETSIZE_MAX);

      // min/max sizes, flags, DND get lost. :-(
      w->reparent(this,0,pnt2,w->isVisible());
      
      w->setMinimumSize(mincs.width(),mincs.height());
      w->setMaximumSize(maxcs.width(),maxcs.height());
      setMinimumWidth(QEXTMDI_MDI_CHILDFRM_MIN_WIDTH);
   } else w->move(QEXTMDI_MDI_CHILDFRM_BORDER,clientYPos);
   
   linkChildren( pFocPolDict);
   
   QObject::connect( m_pClient, SIGNAL(focusInEventOccurs(QextMdiChildView*)), this, SLOT(raiseAndActivate()) );
   QObject::connect( m_pClient, SIGNAL(mdiParentNowMaximized()), m_pManager, SIGNAL(nowMaximized()) );
   QObject::connect( m_pClient, SIGNAL(mdiParentNoLongerMaximized(QextMdiChildFrm*)), m_pManager, SIGNAL(noLongerMaximized(QextMdiChildFrm*)) );
   
   if( m_pClient->minimumSize().width() > m_pManager->m_defaultChildFrmSize.width()) {
      setMinimumWidth(m_pClient->minimumSize().width() + QEXTMDI_MDI_CHILDFRM_DOUBLE_BORDER);
   }
   if( m_pClient->minimumSize().height() > m_pManager->m_defaultChildFrmSize.height()) {
      setMinimumHeight( m_pClient->minimumSize().height()+ QEXTMDI_MDI_CHILDFRM_DOUBLE_BORDER +
                        m_pCaption->heightHint() + QEXTMDI_MDI_CHILDFRM_SEPARATOR);
   }
}

//============ unsetClient ============//

void QextMdiChildFrm::unsetClient( QPoint positionOffset)
{
  if(!m_pClient)return;
   
  QObject::disconnect( m_pClient, SIGNAL(focusInEventOccurs(QextMdiChildView*)), this, SLOT(raiseAndActivate()) );
  QObject::disconnect( m_pClient, SIGNAL(mdiParentNowMaximized()), m_pManager, SIGNAL(nowMaximized()) );
  QObject::disconnect( m_pClient, SIGNAL(mdiParentNoLongerMaximized(QextMdiChildFrm*)), m_pManager, SIGNAL(noLongerMaximized(QextMdiChildFrm*)) );
   
  //reparent to desktop widget , no flags , point , show it
  QDict<FocusPolicy>* pFocPolDict;
  pFocPolDict = unlinkChildren();

   // get name of focused child widget
   QWidget* focusedChildWidget = m_pClient->focusedChildWidget();
   const char* nameOfFocusedWidget = "";
   if( focusedChildWidget != 0)
      nameOfFocusedWidget = focusedChildWidget->name();

   QSize mins = m_pClient->minimumSize();
   QSize maxs = m_pClient->maximumSize();
   m_pClient->reparent(0,0,mapToGlobal(pos())-pos()+positionOffset,isVisible());
   m_pClient->setMinimumSize(mins.width(),mins.height());
   m_pClient->setMaximumSize(maxs.width(),maxs.height());

  // remember the focus policies using the dictionary and reset them
  QObjectList *list = m_pClient->queryList( "QWidget" );
  QObjectListIt it( *list );          // iterate over all child widgets of child frame
  QObject * obj;
  QWidget* firstFocusableChildWidget = 0;
  QWidget* lastFocusableChildWidget = 0;
  while ( (obj=it.current()) != 0 ) { // for each found object...
    QWidget* widg = (QWidget*)obj;
    ++it;
    FocusPolicy* pFocPol = pFocPolDict->find( widg->name()); // remember the focus policy from before the reparent
    if( pFocPol)
       widg->setFocusPolicy( *pFocPol);
    // reset focus to old position (doesn't work :-( for its own unexplicable reasons)
    if( widg->name() == nameOfFocusedWidget) {
      widg->setFocus();
    }
    // get first and last focusable widget
    if( (widg->focusPolicy() == QWidget::StrongFocus) || (widg->focusPolicy() == QWidget::TabFocus)) {
      if( firstFocusableChildWidget == 0)
   firstFocusableChildWidget = widg;  // first widget
      lastFocusableChildWidget = widg; // last widget
      //qDebug("*** %s (%s)",widg->name(),widg->className());
    }
    else {
      if( widg->focusPolicy() == QWidget::WheelFocus) {
   if( firstFocusableChildWidget == 0)
     firstFocusableChildWidget = widg;  // first widget
   lastFocusableChildWidget = widg; // last widget
   //qDebug("*** %s (%s)",widg->name(),widg->className());
      }
    }
  }
  delete list;                        // delete the list, not the objects
  delete pFocPolDict;

  // reset first and last focusable widget
  m_pClient->setFirstFocusableChildWidget( firstFocusableChildWidget);
  m_pClient->setLastFocusableChildWidget( lastFocusableChildWidget);

  // reset the focus policy of the view
  m_pClient->setFocusPolicy(QWidget::ClickFocus);

  // lose information about the view (because it's undocked now)
  m_pClient=0;
}

//============== linkChildren =============//
void QextMdiChildFrm::linkChildren( QDict<FocusPolicy>* pFocPolDict)
{
   // reset the focus policies for all widgets in the view (take them from the dictionary)
   QObjectList *list = m_pClient->queryList( "QWidget" );
   QObjectListIt it( *list );          // iterate over all child widgets of child frame
   QObject * obj;
   while ( (obj=it.current()) != 0 ) { // for each found object...
      QWidget* widg = (QWidget*)obj;
      ++it;
      FocusPolicy* pFocPol = pFocPolDict->find(widg->name()); // remember the focus policy from before the reparent
      if( pFocPol != 0)
         widg->setFocusPolicy( *pFocPol);
      widg->installEventFilter(this);
   }
   delete list;                        // delete the list, not the objects
   delete pFocPolDict;

   // reset the focus policies for the rest
   m_pIcon->setFocusPolicy(QWidget::NoFocus);
   m_pClient->setFocusPolicy(QWidget::ClickFocus);
   m_pCaption->setFocusPolicy(QWidget::NoFocus);
   m_pUndock->setFocusPolicy(QWidget::NoFocus);
   m_pMinimize->setFocusPolicy(QWidget::NoFocus);
   m_pMaximize->setFocusPolicy(QWidget::NoFocus);
   m_pClose->setFocusPolicy(QWidget::NoFocus);

   // install the event filter (catch mouse clicks) for the rest
   m_pIcon->installEventFilter(this);
   m_pCaption->installEventFilter(this);
   m_pUndock->installEventFilter(this);
   m_pMinimize->installEventFilter(this);
   m_pMaximize->installEventFilter(this);
   m_pClose->installEventFilter(this);
   m_pClient->installEventFilter(this);
   m_pClient->installEventFilterForAllChildren();
}

//============== unlinkChildren =============//

QDict<QWidget::FocusPolicy>* QextMdiChildFrm::unlinkChildren()
{
   // memorize the focuses in a dictionary because they will get lost during reparenting
   QDict<FocusPolicy>* pFocPolDict = new QDict<FocusPolicy>;
   pFocPolDict->setAutoDelete(TRUE);

   QObjectList *list = m_pClient->queryList( "QWidget" );
   QObjectListIt it( *list );          // iterate over all child widgets of child frame
   QObject * obj;
   int i = 1;
   while ( (obj=it.current()) != 0 ) { // for each found object...
      ++it;
      QWidget* widg = (QWidget*)obj;
      // get current widget object name
      if( widg->name(0) == 0) {
         QString tmpStr;
         tmpStr.setNum( i);
         tmpStr = "unnamed" + tmpStr;
         widg->setName( tmpStr);
         i++;
      }
      FocusPolicy* pFocPol = new FocusPolicy;
      *pFocPol = widg->focusPolicy();
      // memorize focus policy
      pFocPolDict->insert( widg->name(), pFocPol);
      // remove event filter
      ((QWidget*)obj)->removeEventFilter(this);
   }
   delete list;                        // delete the list, not the objects

   // remove the event filter (catch mouse clicks) for the rest
   m_pIcon->removeEventFilter(this);
   m_pCaption->removeEventFilter(this);
   m_pUndock->removeEventFilter(this);
   m_pMinimize->removeEventFilter(this);
   m_pMaximize->removeEventFilter(this);
   m_pClose->removeEventFilter(this);
   m_pClient->removeEventFilter(this);

   m_pClient->removeEventFilterForAllChildren();

   return pFocPolDict;
}

//============== resizeEvent ===============//

void QextMdiChildFrm::resizeEvent(QResizeEvent *)
{
   //Resize the caption
   int captionHeight=m_pCaption->heightHint();
   int captionWidth=width()-QEXTMDI_MDI_CHILDFRM_DOUBLE_BORDER;
   m_pCaption->setGeometry( QEXTMDI_MDI_CHILDFRM_BORDER, QEXTMDI_MDI_CHILDFRM_BORDER, captionWidth, captionHeight);
   //The buttons are caption children
#ifdef _OS_WIN32_
   m_pIcon->setGeometry(1,1,captionHeight-2,captionHeight-2);
   m_pClose->setGeometry((captionWidth-captionHeight)+1,1,captionHeight-2,captionHeight-2);
   m_pMaximize->setGeometry((captionWidth-(captionHeight*2))+2,1,captionHeight-2,captionHeight-2);
   m_pMinimize->setGeometry((captionWidth-(captionHeight*3))+3,1,captionHeight-2,captionHeight-2);
   m_pUndock->setGeometry((captionWidth-(captionHeight*4))+4,1,captionHeight-2,captionHeight-2);
#else // in case of Unix : KDE look
   m_pIcon->setGeometry(0,0,captionHeight,captionHeight);
   m_pClose->setGeometry((captionWidth-captionHeight),0,captionHeight,captionHeight);
   m_pMaximize->setGeometry((captionWidth-(captionHeight*2)),0,captionHeight,captionHeight);
   m_pMinimize->setGeometry((captionWidth-(captionHeight*3)),0,captionHeight,captionHeight);
   m_pUndock->setGeometry((captionWidth-(captionHeight*4)),0,captionHeight,captionHeight);
#endif
   //Resize the client
   if(m_pClient) {
      QSize newClientSize(captionWidth,
      height()-(QEXTMDI_MDI_CHILDFRM_DOUBLE_BORDER+captionHeight+QEXTMDI_MDI_CHILDFRM_SEPARATOR));
      if (newClientSize != m_pClient->size()) {
         m_pClient->resize(newClientSize.width(), newClientSize.height());
      }
   }
}

//============= eventFilter ===============//

bool QextMdiChildFrm::eventFilter( QObject *obj, QEvent *e )
{
   if ( (e->type() == QEvent::MouseButtonPress) && (((QWidget*)obj) != m_pClient) ) {
      raiseAndActivate();
      QWidget* w = (QWidget*) obj;
      if( (w->parent() != m_pCaption) && (w != m_pCaption))
         w->setFocus();
   }
   else if( (e->type() == QEvent::Resize) && (((QWidget*)obj) == m_pClient) ) {
      QResizeEvent* re = (QResizeEvent*)e;
      int captionHeight = m_pCaption->heightHint();
      QSize newChildFrmSize( re->size().width() + QEXTMDI_MDI_CHILDFRM_DOUBLE_BORDER,
                             re->size().height() + captionHeight + QEXTMDI_MDI_CHILDFRM_SEPARATOR + QEXTMDI_MDI_CHILDFRM_DOUBLE_BORDER );
      if( newChildFrmSize != size())
         resize( newChildFrmSize );
   }
   return FALSE;                           // standard event processing
}

//============= raiseAndActivate ===============//

void QextMdiChildFrm::raiseAndActivate()
{
   //qDebug("ChildFrm::raiseAndActivate");
   m_pCaption->setActive(TRUE);
   m_pManager->setTopChild(this,FALSE); //Do not focus by now...
}

QPopupMenu* QextMdiChildFrm::systemMenu()
{
   if( m_pSystemMenu == 0)
      return 0;

   m_pSystemMenu->clear();

#ifdef _OS_WIN32_
   m_pSystemMenu->insertItem(tr("&Restore"),this,SLOT(restorePressed()));
   m_pSystemMenu->insertItem(tr("&Move"),m_pCaption, SLOT(slot_moveViaSystemMenu()));
   m_pSystemMenu->insertItem(tr("R&esize"),this, SLOT(slot_resizeViaSystemMenu()));
   m_pSystemMenu->insertItem(tr("M&inimize"),this, SLOT(minimizePressed()));
   m_pSystemMenu->insertItem(tr("M&aximize"),this, SLOT(maximizePressed()));
   if( state() == Normal)
      m_pSystemMenu->setItemEnabled(m_pSystemMenu->idAt(0),FALSE);
   else if( state() == Maximized) {
      m_pSystemMenu->setItemEnabled(m_pSystemMenu->idAt(1),FALSE);
      m_pSystemMenu->setItemEnabled(m_pSystemMenu->idAt(2),FALSE);
      m_pSystemMenu->setItemEnabled(m_pSystemMenu->idAt(4),FALSE);
   }
   else if( state() == Minimized) {
      m_pSystemMenu->setItemEnabled(m_pSystemMenu->idAt(2),FALSE);
      m_pSystemMenu->setItemEnabled(m_pSystemMenu->idAt(3),FALSE);
   }
#else
   if( state() != Normal)
      m_pSystemMenu->insertItem(tr("&Restore"),this,SLOT(restorePressed()));
   if( state() != Maximized)
      m_pSystemMenu->insertItem(tr("&Maximize"),this, SLOT(maximizePressed()));
   if( state() != Minimized)
      m_pSystemMenu->insertItem(tr("&Iconify"),this, SLOT(minimizePressed()));
   if( state() != Maximized)
      m_pSystemMenu->insertItem(tr("M&ove"),m_pCaption, SLOT(slot_moveViaSystemMenu()));
   if( state() == Normal)
      m_pSystemMenu->insertItem(tr("&Resize"),this, SLOT(slot_resizeViaSystemMenu()));
#endif

   m_pSystemMenu->insertItem(tr("&Undock"),this, SLOT(undockPressed()));
   m_pSystemMenu->insertSeparator();
   m_pSystemMenu->insertItem(tr("&Close"),this, SLOT(closePressed()));

   return m_pSystemMenu;
}

/** Shows a system menu for child frame windows. */
void QextMdiChildFrm::showSystemMenu()
{
#ifndef _OS_WIN32_
   m_pIcon->setDown( FALSE);
#endif
   QPoint popupmenuPosition;
   //qDebug("%d,%d,%d,%d,%d",m_pIcon->pos().x(),x(),m_pIcon->pos().y(),m_pIcon->height(),y());
   popupmenuPosition = QPoint( m_pIcon->pos().x(),
                               m_pIcon->pos().y() + m_pIcon->height() + QEXTMDI_MDI_CHILDFRM_BORDER );
   systemMenu()->popup( mapToGlobal( popupmenuPosition));
}

void QextMdiChildFrm::switchToMinimizeLayout()
{
   setMinimumWidth(QEXTMDI_MDI_CHILDFRM_MIN_WIDTH);
   setFixedHeight(m_pCaption->height()+QEXTMDI_MDI_CHILDFRM_DOUBLE_BORDER+QEXTMDI_MDI_CHILDFRM_SEPARATOR);

   m_pMaximize->setPixmap( *m_pMaxButtonPixmap);

   // temporary use of minimize button for restore function
   m_pMinimize->setPixmap( *m_pRestoreButtonPixmap);
   QObject::disconnect(m_pMinimize,SIGNAL(clicked()),this,SLOT(minimizePressed()));
   QObject::connect(m_pMinimize,SIGNAL(clicked()),this,SLOT(restorePressed()));

   // resizing
   resize( 300, minimumHeight());

   // positioning
   m_pManager->layoutMinimizedChildren();
}

void QextMdiChildFrm::slot_resizeViaSystemMenu()
{
   grabMouse();
   m_resizeMode = TRUE;
   m_iResizeCorner = QEXTMDI_RESIZE_BOTTOMLEFT;
   setResizeCursor( m_iResizeCorner);
}

#include "moc_qextmdichildfrm.cpp"
