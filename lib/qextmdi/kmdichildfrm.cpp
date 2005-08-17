//----------------------------------------------------------------------------
//    filename             : kmdichildfrm.cpp
//----------------------------------------------------------------------------
//    Project              : KDE MDI extension
//
//    begin                : 07/1999       by Szymon Stefanek as part of kvirc
//                                         (an IRC application)
//    changes              : 09/1999       by Falk Brettschneider to create an
//                           - 06/2000     stand-alone Qt extension set of
//                                         classes and a Qt-based library
//                         : 01/2003       by Jens Zurheide to allow switching
//                                         between views based on timestamps
//                           2000-2003     maintained by the KDevelop project
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

#include "kmdichildfrm.h"
#include "kmdichildfrm.moc"

#include "kmdidefines.h"
#include "kmdichildfrmcaption.h"
#include "kmdichildarea.h"
#include "kmdimainfrm.h"

#include <qpainter.h>
#include <qapplication.h>
#include <qcursor.h>
#include <qobject.h>
#include <q3frame.h>
#include <q3popupmenu.h>
#include <qtoolbutton.h>
#include <qnamespace.h>
#include <qimage.h>
//Added by qt3to4:
#include <QPixmap>
#include <QFocusEvent>
#include <QChildEvent>
#include <QMoveEvent>
#include <QEvent>
#include <QLabel>
#include <QResizeEvent>
#include <QMouseEvent>

#include <klocale.h>
#include <kiconloader.h>

//////////////////////////////////////////////////////////////////////////////
// KMdiChildFrm
//////////////////////////////////////////////////////////////////////////////

#define KMDI_NORESIZE 0
#define KMDI_RESIZE_TOP 1
#define KMDI_RESIZE_LEFT 2
#define KMDI_RESIZE_RIGHT 4
#define KMDI_RESIZE_BOTTOM 8
#define KMDI_RESIZE_TOPLEFT (1|2)
#define KMDI_RESIZE_TOPRIGHT (1|4)
#define KMDI_RESIZE_BOTTOMLEFT (8|2)
#define KMDI_RESIZE_BOTTOMRIGHT (8|4)

#include "filenew.xpm"
#include "win_closebutton.xpm"
#include "win_minbutton.xpm"
#include "win_maxbutton.xpm"
#include "win_restorebutton.xpm"
#include "win_undockbutton.xpm"
#include "kde_closebutton.xpm"
#include "kde_minbutton.xpm"
#include "kde_maxbutton.xpm"
#include "kde_restorebutton.xpm"
#include "kde_undockbutton.xpm"
#include "kde2_closebutton.xpm"
#include "kde2_minbutton.xpm"
#include "kde2_maxbutton.xpm"
#include "kde2_restorebutton.xpm"
#include "kde2_undockbutton.xpm"
#include "kde2laptop_closebutton.xpm"
#include "kde2laptop_minbutton.xpm"
#include "kde2laptop_maxbutton.xpm"
#include "kde2laptop_restorebutton.xpm"
#include "kde2laptop_undockbutton.xpm"


KMdiWin32IconButton::KMdiWin32IconButton( QWidget* parent, const char* name)
  : QLabel( parent, name)
{
}

//============ mousePressEvent ============//

void KMdiWin32IconButton::mousePressEvent( QMouseEvent*)
{
   //emit pressed();
}

//============ KMdiChildFrm ============//

KMdiChildFrm::KMdiChildFrm(KMdiChildArea *parent)
 : Q3Frame(parent, "kmdi_childfrm")
   ,m_pClient(0L)
   ,m_pManager(0L)
   ,m_pCaption(0L)
   ,m_pWinIcon(0L)
   ,m_pUnixIcon(0L)
   ,m_pMinimize(0L)
   ,m_pMaximize(0L)
   ,m_pClose(0L)
   ,m_pUndock(0L)
   ,m_state(Normal)
   ,m_restoredRect()
   ,m_iResizeCorner(KMDI_NORESIZE)
   ,m_iLastCursorCorner(KMDI_NORESIZE)
   ,m_bResizing(false)
   ,m_bDragging(false)
   ,m_pIconButtonPixmap(0L)
   ,m_pMinButtonPixmap(0L)
   ,m_pMaxButtonPixmap(0L)
   ,m_pRestoreButtonPixmap(0L)
   ,m_pCloseButtonPixmap(0L)
   ,m_pUndockButtonPixmap(0L)
   ,m_windowMenuID(0)
   ,m_pSystemMenu(0L)
   ,m_oldClientMinSize()
   ,m_oldClientMaxSize()
   ,m_oldLayoutResizeMode(QLayout::SetMinimumSize)
{
   m_pCaption  = new KMdiChildFrmCaption(this);

   m_pManager  = parent;

   m_pWinIcon  = new KMdiWin32IconButton(m_pCaption, "kmdi_iconbutton_icon");
   m_pUnixIcon = new QToolButton(m_pCaption, "kmdi_toolbutton_icon");
   m_pMinimize = new QToolButton(m_pCaption, "kmdi_toolbutton_min");
   m_pMaximize = new QToolButton(m_pCaption, "kmdi_toolbutton_max");
   m_pClose    = new QToolButton(m_pCaption, "kmdi_toolbutton_close");
   m_pUndock   = new QToolButton(m_pCaption, "kmdi_toolbutton_undock");

   QObject::connect(m_pMinimize,SIGNAL(clicked()),this,SLOT(minimizePressed()));
   QObject::connect(m_pMaximize,SIGNAL(clicked()),this,SLOT(maximizePressed()));
   QObject::connect(m_pClose,SIGNAL(clicked()),this,SLOT(closePressed()));
   QObject::connect(m_pUndock,SIGNAL(clicked()),this,SLOT(undockPressed()));

   m_pIconButtonPixmap = new QPixmap( SmallIcon("filenew") );
   if (m_pIconButtonPixmap->isNull())
      *m_pIconButtonPixmap = QPixmap(filenew);
   
   redecorateButtons();

   m_pWinIcon->setFocusPolicy(Qt::NoFocus);
   m_pUnixIcon->setFocusPolicy(Qt::NoFocus);
   m_pClose->setFocusPolicy(Qt::NoFocus);
   m_pMinimize->setFocusPolicy(Qt::NoFocus);
   m_pMaximize->setFocusPolicy(Qt::NoFocus);
   m_pUndock->setFocusPolicy(Qt::NoFocus);

   setFrameStyle(Q3Frame::WinPanel|Q3Frame::Raised);
   setFocusPolicy(Qt::NoFocus);

   setMouseTracking(true);

   setMinimumSize(KMDI_CHILDFRM_MIN_WIDTH, m_pCaption->heightHint());

   m_pSystemMenu = new Q3PopupMenu();
}

//============ ~KMdiChildFrm ============//

KMdiChildFrm::~KMdiChildFrm()
{
   delete m_pMinButtonPixmap;
   delete m_pMaxButtonPixmap;
   delete m_pRestoreButtonPixmap;
   delete m_pCloseButtonPixmap;
   delete m_pUndockButtonPixmap;
   delete m_pSystemMenu;
   delete m_pIconButtonPixmap;
}

//============ mousePressEvent =============//
void KMdiChildFrm::mousePressEvent(QMouseEvent *e)
{
   if( m_bResizing) {
      if(QApplication::overrideCursor()) { QApplication::restoreOverrideCursor(); }
      m_bResizing = false;
      releaseMouse();
   }

   m_pCaption->setActive(true);
   m_pManager->setTopChild(this,false);

   m_iResizeCorner=getResizeCorner(e->pos().x(),e->pos().y());
   if(m_iResizeCorner != KMDI_NORESIZE) {
      m_bResizing = true;
      //notify child view
      KMdiChildFrmResizeBeginEvent ue(e);
      if( m_pClient != 0L) {
         QApplication::sendEvent( m_pClient, &ue);
      }
   }
}

//============ mouseReleaseEvent ==============//

void KMdiChildFrm::mouseReleaseEvent(QMouseEvent *e)
{
   if(m_bResizing) {
      if(QApplication::overrideCursor()) { QApplication::restoreOverrideCursor(); }

      m_bResizing = false;
      //notify child view
      KMdiChildFrmResizeEndEvent ue(e);
      if( m_pClient != 0L) {
         QApplication::sendEvent( m_pClient, &ue);
      }
   }
}

//============= setResizeCursor ===============//

void KMdiChildFrm::setResizeCursor(int resizeCorner)
{
   if(resizeCorner == m_iLastCursorCorner)return; //Don't do it twice
   m_iLastCursorCorner = resizeCorner;
   switch (resizeCorner) {
      case KMDI_NORESIZE:
         if(QApplication::overrideCursor())QApplication::restoreOverrideCursor();
         break;
      case KMDI_RESIZE_LEFT:
      case KMDI_RESIZE_RIGHT:
         QApplication::setOverrideCursor(Qt::SizeHorCursor,true);
         break;
      case KMDI_RESIZE_TOP:
      case KMDI_RESIZE_BOTTOM:
         QApplication::setOverrideCursor(Qt::SizeVerCursor,true);
         break;
      case KMDI_RESIZE_TOPLEFT:
      case KMDI_RESIZE_BOTTOMRIGHT:
         QApplication::setOverrideCursor(Qt::SizeFDiagCursor,true);
         break;
      case KMDI_RESIZE_BOTTOMLEFT:
      case KMDI_RESIZE_TOPRIGHT:
         QApplication::setOverrideCursor(Qt::SizeBDiagCursor,true);
         break;
   }
}

//============= unsetResizeCursor ===============//

void KMdiChildFrm::unsetResizeCursor()
{
   if ( !m_bResizing && (m_iResizeCorner != KMDI_NORESIZE) ) {
      m_iResizeCorner=KMDI_NORESIZE;
      m_iLastCursorCorner=KMDI_NORESIZE;
      if(QApplication::overrideCursor())QApplication::restoreOverrideCursor();
   }
}

//============= mouseMoveEvent ===============//

void KMdiChildFrm::mouseMoveEvent(QMouseEvent *e)
{
   if(m_state != Normal) return;
   if(!m_pClient) return;
   if(m_pClient->minimumSize() == m_pClient->maximumSize()) return;

   if(m_bResizing) {
      if( !(e->state() & Qt::RightButton) && !(e->state() & Qt::MidButton)) {
         // same as: if no button or left button pressed
         QPoint p = parentWidget()->mapFromGlobal( e->globalPos() );
         resizeWindow(m_iResizeCorner, p.x(), p.y());
      }
      else
         m_bResizing = false;
   }
   else {
      m_iResizeCorner = getResizeCorner(e->pos().x(), e->pos().y());
      setResizeCursor( m_iResizeCorner);
   }
}

//============= moveEvent ===============//

void KMdiChildFrm::moveEvent(QMoveEvent* me)
{
   // give its child view the chance to notify a childframe move
   KMdiChildFrmMoveEvent cfme( me);
   if( m_pClient != 0L) {
      QApplication::sendEvent( m_pClient, &cfme);
   }
}

//=============== leaveEvent ===============//

void KMdiChildFrm::leaveEvent(QEvent *)
{
   unsetResizeCursor();
}

void KMdiChildFrm::resizeWindow(int resizeCorner, int xPos, int yPos)
{
   QRect resizeRect(x(),y(),width(),height());

   // Calculate the minimum width & height
   int minWidth=0;
   int minHeight=0;
   int maxWidth=QWIDGETSIZE_MAX;
   int maxHeight=QWIDGETSIZE_MAX;
   // it could be the client forces the childframe to enlarge its minimum size
   if(m_pClient){
      minWidth  = m_pClient->minimumSize().width() + KMDI_CHILDFRM_DOUBLE_BORDER;
      minHeight = m_pClient->minimumSize().height()+ KMDI_CHILDFRM_DOUBLE_BORDER +
                    m_pCaption->heightHint() + KMDI_CHILDFRM_SEPARATOR;
      maxWidth  = m_pClient->maximumSize().width() + KMDI_CHILDFRM_DOUBLE_BORDER;
      maxHeight = m_pClient->maximumSize().height()+ KMDI_CHILDFRM_DOUBLE_BORDER +
                    m_pCaption->heightHint() + KMDI_CHILDFRM_SEPARATOR;
   }
   if(minWidth<minimumWidth())minWidth=minimumWidth();
   if(minHeight<minimumHeight())minHeight=minimumHeight();
   if(maxWidth>maximumWidth())maxWidth=maximumWidth();
   if(maxHeight>maximumHeight())maxHeight=maximumHeight();

   QPoint mousePos( xPos, yPos);

   // manipulate width
   switch (resizeCorner){
   case KMDI_RESIZE_TOPLEFT:     // no break
   case KMDI_RESIZE_LEFT:        // no break
   case KMDI_RESIZE_BOTTOMLEFT:
      resizeRect.setLeft(mousePos.x());
      if(resizeRect.width() < minWidth)resizeRect.setLeft(resizeRect.right() - minWidth + 1);
      if(resizeRect.width() > maxWidth)resizeRect.setLeft(resizeRect.right() - maxWidth + 1);
      break;
   case KMDI_RESIZE_TOPRIGHT:    // no break
   case KMDI_RESIZE_RIGHT:       // no break
   case KMDI_RESIZE_BOTTOMRIGHT:
      resizeRect.setRight(mousePos.x());
      if(resizeRect.width() < minWidth)resizeRect.setRight(resizeRect.left() + minWidth - 1);
      if(resizeRect.width() > maxWidth)resizeRect.setRight(resizeRect.left() + maxWidth - 1);
      break;
   default:
      // nothing to do
      break;
   }
   // manipulate height
   switch (resizeCorner){
   case KMDI_RESIZE_TOPLEFT:  // no break
   case KMDI_RESIZE_TOP:      // no break
   case KMDI_RESIZE_TOPRIGHT:
      resizeRect.setTop(mousePos.y());
      if(resizeRect.height() < minHeight)resizeRect.setTop(resizeRect.bottom() - minHeight + 1);
      if(resizeRect.height() > maxHeight)resizeRect.setTop(resizeRect.bottom() - maxHeight + 1);
      break;
   case KMDI_RESIZE_BOTTOMLEFT:  // no break
   case KMDI_RESIZE_BOTTOM:      // no break
   case KMDI_RESIZE_BOTTOMRIGHT:
      resizeRect.setBottom(mousePos.y());
      if(resizeRect.height() < minHeight)resizeRect.setBottom(resizeRect.top() + minHeight - 1);
      if(resizeRect.height() > maxHeight)resizeRect.setBottom(resizeRect.top() + maxHeight - 1);
      break;
   default:
      // nothing to do
      break;
   }
   // actually resize
   setGeometry( resizeRect);

   if(m_state==Maximized){
      m_state=Normal;
      m_pMaximize->setPixmap( *m_pMaxButtonPixmap);
   }
}

//================= getResizeCorner =============//

int KMdiChildFrm::getResizeCorner(int ax,int ay)
{
   int ret = KMDI_NORESIZE;
   if(m_pClient->minimumWidth() != m_pClient->maximumWidth()) {
   if((ax>0)&&(ax<(KMDI_CHILDFRM_BORDER+2))) ret |= KMDI_RESIZE_LEFT;
     if((ax<width())&&(ax>(width()-(KMDI_CHILDFRM_BORDER+2)))) ret |= KMDI_RESIZE_RIGHT;
   }
  if(m_pClient->minimumHeight() != m_pClient->maximumHeight()) {
   if((ay>0)&&(ay<(KMDI_CHILDFRM_BORDER+2))) ret |= KMDI_RESIZE_TOP;
     if((ay<(height()))&&(ay>(height()-(KMDI_CHILDFRM_BORDER+2)))) ret |= KMDI_RESIZE_BOTTOM;
   }
   return ret;
}

//============= maximizePressed ============//

void KMdiChildFrm::maximizePressed()
{
   switch(m_state){
      case Maximized:
         emit m_pManager->nowMaximized(false);
         setState(Normal);
         break;
      case Normal:
      case Qt::DockMinimized:
         setState(Maximized);
         emit m_pManager->nowMaximized(true);
         break;
   }
}

void KMdiChildFrm::restorePressed()
{
   if( m_state == Normal)
      return;
   if( m_state == Maximized)
      emit m_pManager->nowMaximized(false);
   setState(Normal);
}

//============= minimizePressed ============//

void KMdiChildFrm::minimizePressed()
{
   switch(m_state){
      case Qt::DockMinimized: setState(Normal);    break;
      case Normal:    setState(Qt::DockMinimized); break;
      case Maximized:
         emit m_pManager->nowMaximized(false);
         setState(Normal);
         setState(Qt::DockMinimized);
         break;
   }
}

//============= closePressed ============//

void KMdiChildFrm::closePressed()
{
   if(m_pClient)
      m_pClient->close();
}

//============= undockPressed ============//

void KMdiChildFrm::undockPressed()
{
   if(m_pClient) {
      if( m_state == Qt::DockMinimized)
         setState( Normal);
      m_pClient->detach();
   }
}

//============ setState =================//

void KMdiChildFrm::setState(MdiWindowState state, bool /*bAnimate*/)
{
   if(m_state==Normal){ //save the current rect
      m_restoredRect=QRect(x(),y(),width(),height());
   }
   switch(state){
   case Normal:
      switch(m_state){
      case Maximized:
         m_pClient->m_stateChanged = true;
         m_state=state;
         // client min / max size / layout behavior don't change
         // set frame max size indirectly by setting the clients max size to
         // it's current value (calls setMaxSize() of frame)
         m_pClient->setMaximumSize(m_pClient->maximumSize().width(), m_pClient->maximumSize().height());
         m_pMaximize->setPixmap( *m_pMaxButtonPixmap);
         setGeometry(m_restoredRect);
         break;
      case Qt::DockMinimized:
         m_pClient->m_stateChanged = true;
         m_state=state;
         // restore client min / max size / layout behavior
         m_pClient->setMinimumSize(m_oldClientMinSize.width(),m_oldClientMinSize.height());
         m_pClient->setMaximumSize(m_oldClientMaxSize.width(),m_oldClientMaxSize.height());
         if (m_pClient->layout() != 0L) {
            m_pClient->layout()->setResizeMode(m_oldLayoutResizeMode);
         }
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
      switch(m_state){
      case Qt::DockMinimized: {
            m_pClient->m_stateChanged = true;
            m_state=state;
            // restore client min / max size / layout behavior
            m_pClient->setMinimumSize(m_oldClientMinSize.width(),m_oldClientMinSize.height());
            m_pClient->setMaximumSize(m_oldClientMaxSize.width(),m_oldClientMaxSize.height());
            if (m_pClient->layout() != 0L) {
               m_pClient->layout()->setResizeMode(m_oldLayoutResizeMode);
            }
            setMaximumSize(QWIDGETSIZE_MAX,QWIDGETSIZE_MAX);
            // reset to maximize-captionbar
            m_pMaximize->setPixmap( *m_pRestoreButtonPixmap);
            m_pMinimize->setPixmap( *m_pMinButtonPixmap);
            QObject::disconnect(m_pMinimize,SIGNAL(clicked()),this,SLOT(restorePressed()));
            QObject::connect(m_pMinimize,SIGNAL(clicked()),this,SLOT(minimizePressed()));
            int nFrameWidth = KMDI_CHILDFRM_DOUBLE_BORDER;
            int nFrameHeight = KMDI_CHILDFRM_DOUBLE_BORDER + KMDI_CHILDFRM_SEPARATOR +
                               m_pCaption->heightHint();
            setGeometry(-m_pClient->x(), -m_pClient->y(),
                        m_pManager->width() + nFrameWidth,
                        m_pManager->height() + nFrameHeight);
            raise();
         }
         break;
      case Normal: {
            m_pClient->m_stateChanged = true;
            m_state=state;
            // client min / max size / layout behavior don't change
            setMaximumSize(QWIDGETSIZE_MAX,QWIDGETSIZE_MAX);
            m_pMaximize->setPixmap( *m_pRestoreButtonPixmap);
            int nFrameWidth = KMDI_CHILDFRM_DOUBLE_BORDER;
            int nFrameHeight = KMDI_CHILDFRM_DOUBLE_BORDER + KMDI_CHILDFRM_SEPARATOR +
                               m_pCaption->heightHint();
            QRect maximizedFrmRect(-m_pClient->x(), -m_pClient->y(),
                                   m_pManager->width() + nFrameWidth,
                                   m_pManager->height() + nFrameHeight);
            if (geometry() != maximizedFrmRect) {
               setGeometry(maximizedFrmRect);
            }
            raise();
         }
         break;
      case Maximized:
         break;
      }
   break;
   case Qt::DockMinimized:
      switch(m_state){
      case Maximized:
         m_pClient->m_stateChanged = true;
         m_state=state;
         // save client min / max size / layout behavior
         m_oldClientMinSize = m_pClient->minimumSize();
         m_oldClientMaxSize = m_pClient->maximumSize();
         if (m_pClient->layout() != 0L) {
            m_oldLayoutResizeMode = m_pClient->layout()->resizeMode();
         }
         m_pClient->setMinimumSize(0, 0);
         m_pClient->setMaximumSize(0, 0);
         if (m_pClient->layout() != 0L) {
            m_pClient->layout()->setResizeMode(QLayout::SetNoConstraint);
         }
         switchToMinimizeLayout();
         m_pManager->childMinimized(this,true);
         break;
      case Normal:
         m_pClient->m_stateChanged = true;
         m_state=state;
         // save client min / max size / layout behavior
         m_oldClientMinSize = m_pClient->minimumSize();
         m_oldClientMaxSize = m_pClient->maximumSize();
         if (m_pClient->layout() != 0L) {
            m_oldLayoutResizeMode = m_pClient->layout()->resizeMode();
         }
         m_restoredRect = geometry();
         m_pClient->setMinimumSize(0, 0);
         m_pClient->setMaximumSize(0, 0);
         if (m_pClient->layout() != 0L) {
            m_pClient->layout()->setResizeMode(QLayout::SetNoConstraint);
         }
         switchToMinimizeLayout();
         m_pManager->childMinimized(this,false);
         break;
      case Qt::DockMinimized:
         break;
      }
   break;
   }

   KMdiChildFrm*     pTopFrame = m_pManager->topChild();
   KMdiChildView*    pTopChild = 0L;
   if (pTopFrame != 0L) {
      pTopChild = pTopFrame->m_pClient;
   }
   if ( (pTopChild != 0L) && pTopChild->isMaximized() ) {
      m_pManager->setMinimumSize(pTopChild->minimumWidth(), pTopChild->minimumHeight());
      /// @todo: setting the maximum size doesn't work properly - fix this later
      /// m_pManager->setMaximumSize(pTopChild->maximumWidth(), pTopChild->maximumHeight());
   }
   else {
      m_pManager->setMinimumSize(0, 0);
      m_pManager->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
   }
}

//============== restoreGeometry ================//

QRect KMdiChildFrm::restoreGeometry() const
{
   return m_restoredRect;
}

//============== setRestoreGeometry ================//

void KMdiChildFrm::setRestoreGeometry(const QRect& newRestGeo)
{
   m_restoredRect = newRestGeo;
}

//============ setCaption ===============//

void KMdiChildFrm::setCaption(const QString& text)
{
   m_pCaption->setCaption(text);
}

//============ enableClose ==============//

void KMdiChildFrm::enableClose(bool bEnable)
{
   m_pClose->setEnabled(bEnable);
   m_pClose->repaint(false);
}

//============ setIcon ==================//

void KMdiChildFrm::setIcon(const QPixmap& pxm)
{
   QPixmap p = pxm;
   if (p.width()!=18 || p.height()!=18) {
      QImage img = p.convertToImage();
      p = img.smoothScale(18,18,Qt::KeepAspectRatio);
   }
   const bool do_resize = m_pIconButtonPixmap->size()!=p.size();
   *m_pIconButtonPixmap = p;
   m_pWinIcon->setPixmap( p );
   m_pUnixIcon->setPixmap( p );
   if (do_resize)
      doResize(true);
}

//============ icon =================//

QPixmap* KMdiChildFrm::icon() const
{
   return m_pIconButtonPixmap;
}

//============ setClient ============//
void KMdiChildFrm::setClient(KMdiChildView *w, bool bAutomaticResize)
{
   m_pClient=w;

   if (w->icon())
      setIcon( *(w->icon()));

   //resize to match the client
   int clientYPos=m_pCaption->heightHint()+KMDI_CHILDFRM_SEPARATOR+KMDI_CHILDFRM_BORDER;
   if (bAutomaticResize || w->size().isEmpty() || (w->size() == QSize(1,1))) {
      if (m_pManager->topChild()) {
         resize(m_pManager->topChild()->size());
      }
      else {
         resize(m_pManager->m_defaultChildFrmSize.width()+KMDI_CHILDFRM_DOUBLE_BORDER,m_pManager->m_defaultChildFrmSize.height()+KMDI_CHILDFRM_BORDER+clientYPos);
      }
   }
   else {
      resize(w->width()+KMDI_CHILDFRM_DOUBLE_BORDER,w->height()+KMDI_CHILDFRM_BORDER+clientYPos);
   }

   // memorize the focuses in a dictionary because they will get lost during reparenting
   Q3Dict<Qt::FocusPolicy>* pFocPolDict = new Q3Dict<Qt::FocusPolicy>;
   pFocPolDict->setAutoDelete(true);
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
         widg->setName( tmpStr.latin1() );
         i++;
      }
      Qt::FocusPolicy* pFocPol = new Qt::FocusPolicy;
      *pFocPol = widg->focusPolicy();
      pFocPolDict->insert( widg->name(), pFocPol);
   }
   delete list;                        // delete the list, not the objects

   //Reparent if needed
   if(w->parent()!=this){
      //reparent to this widget , no flags , point , show it
      QPoint pnt2(KMDI_CHILDFRM_BORDER,clientYPos);
      QSize mincs = w->minimumSize();
      QSize maxcs = w->maximumSize();
      w->setMinimumSize(0,0);
      w->setMaximumSize(QWIDGETSIZE_MAX,QWIDGETSIZE_MAX);

      // min/max sizes, flags, DND get lost. :-(
      w->reparent(this,0,pnt2,w->isVisible());

      w->setMinimumSize(mincs.width(),mincs.height());
      w->setMaximumSize(maxcs.width(),maxcs.height());
   } else w->move(KMDI_CHILDFRM_BORDER,clientYPos);

   linkChildren( pFocPolDict);

   QObject::connect( m_pClient, SIGNAL(mdiParentNowMaximized(bool)), m_pManager, SIGNAL(nowMaximized(bool)) );

   if( m_pClient->minimumSize().width() > m_pManager->m_defaultChildFrmSize.width()) {
      setMinimumWidth(m_pClient->minimumSize().width() + KMDI_CHILDFRM_DOUBLE_BORDER);
   }
   if( m_pClient->minimumSize().height() > m_pManager->m_defaultChildFrmSize.height()) {
      setMinimumHeight( m_pClient->minimumSize().height()+ KMDI_CHILDFRM_DOUBLE_BORDER +
                        m_pCaption->heightHint() + KMDI_CHILDFRM_SEPARATOR);
   }
}

//============ unsetClient ============//

void KMdiChildFrm::unsetClient( QPoint positionOffset)
{
   if(!m_pClient)return;

   QObject::disconnect( m_pClient, SIGNAL(mdiParentNowMaximized(bool)), m_pManager, SIGNAL(nowMaximized(bool)) );

   //reparent to desktop widget , no flags , point , show it
   Q3Dict<Qt::FocusPolicy>* pFocPolDict;
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
      Qt::FocusPolicy* pFocPol = pFocPolDict->find( widg->name()); // remember the focus policy from before the reparent
      if( pFocPol)
         widg->setFocusPolicy( *pFocPol);
      // reset focus to old position (doesn't work :-( for its own unexplicable reasons)
      if( widg->name() == nameOfFocusedWidget) {
         widg->setFocus();
      }
      // get first and last focusable widget
      if( (widg->focusPolicy() == Qt::StrongFocus) || (widg->focusPolicy() == Qt::TabFocus)) {
         if( firstFocusableChildWidget == 0)
            firstFocusableChildWidget = widg;  // first widget
         lastFocusableChildWidget = widg; // last widget
         //qDebug("*** %s (%s)",widg->name(),widg->className());
      }
      else {
         if( widg->focusPolicy() == Qt::WheelFocus) {
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
   m_pClient->setFocusPolicy(Qt::ClickFocus);

   // lose information about the view (because it's undocked now)
   m_pClient=0;
}

//============== linkChildren =============//
void KMdiChildFrm::linkChildren( Q3Dict<Qt::FocusPolicy>* pFocPolDict)
{
   // reset the focus policies for all widgets in the view (take them from the dictionary)
   QObjectList *list = m_pClient->queryList( "QWidget" );
   QObjectListIt it( *list );          // iterate over all child widgets of child frame
   QObject * obj;
   while ( (obj=it.current()) != 0 ) { // for each found object...
      QWidget* widg = (QWidget*)obj;
      ++it;
      Qt::FocusPolicy* pFocPol = pFocPolDict->find(widg->name()); // remember the focus policy from before the reparent
      if( pFocPol != 0)
         widg->setFocusPolicy( *pFocPol);
      if (!(widg->inherits("QPopupMenu"))) {
         widg->installEventFilter(this);
      }
   }
   delete list;                        // delete the list, not the objects
   delete pFocPolDict;

   // reset the focus policies for the rest
   m_pWinIcon->setFocusPolicy(Qt::NoFocus);
   m_pUnixIcon->setFocusPolicy(Qt::NoFocus);
   m_pClient->setFocusPolicy(Qt::ClickFocus);
   m_pCaption->setFocusPolicy(Qt::NoFocus);
   m_pUndock->setFocusPolicy(Qt::NoFocus);
   m_pMinimize->setFocusPolicy(Qt::NoFocus);
   m_pMaximize->setFocusPolicy(Qt::NoFocus);
   m_pClose->setFocusPolicy(Qt::NoFocus);

   // install the event filter (catch mouse clicks) for the rest
   m_pWinIcon->installEventFilter(this);
   m_pUnixIcon->installEventFilter(this);
   m_pCaption->installEventFilter(this);
   m_pUndock->installEventFilter(this);
   m_pMinimize->installEventFilter(this);
   m_pMaximize->installEventFilter(this);
   m_pClose->installEventFilter(this);
   m_pClient->installEventFilter(this);
//   m_pClient->installEventFilterForAllChildren();
}

//============== unlinkChildren =============//

Q3Dict<Qt::FocusPolicy>* KMdiChildFrm::unlinkChildren()
{
   // memorize the focuses in a dictionary because they will get lost during reparenting
   Q3Dict<Qt::FocusPolicy>* pFocPolDict = new Q3Dict<Qt::FocusPolicy>;
   pFocPolDict->setAutoDelete(true);

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
         widg->setName( tmpStr.latin1() );
         i++;
      }
      Qt::FocusPolicy* pFocPol = new Qt::FocusPolicy;
      *pFocPol = widg->focusPolicy();
      // memorize focus policy
      pFocPolDict->insert( widg->name(), pFocPol);
      // remove event filter
      ((QWidget*)obj)->removeEventFilter(this);
   }
   delete list;                        // delete the list, not the objects

   // remove the event filter (catch mouse clicks) for the rest
   m_pWinIcon->removeEventFilter(this);
   m_pUnixIcon->removeEventFilter(this);
   m_pCaption->removeEventFilter(this);
   m_pUndock->removeEventFilter(this);
   m_pMinimize->removeEventFilter(this);
   m_pMaximize->removeEventFilter(this);
   m_pClose->removeEventFilter(this);
   m_pClient->removeEventFilter(this);

//SCHEDULED_FOR_REMOVE   m_pClient->removeEventFilterForAllChildren();

   return pFocPolDict;
}

//============== resizeEvent ===============//

void KMdiChildFrm::resizeEvent(QResizeEvent *)
{
   doResize(); // an extra method because it can also called directly
}

void KMdiChildFrm::doResize()
{
	doResize(false);
}

void KMdiChildFrm::doResize(bool captionOnly)
{
   //Resize the caption
   int captionHeight = m_pCaption->heightHint();
   int captionWidth = width() - KMDI_CHILDFRM_DOUBLE_BORDER;
   int buttonHeight = m_pClose->pixmap()->height();
   int buttonWidth  = m_pClose->pixmap()->width();
   int heightOffset = captionHeight/2 - buttonHeight/2;
   int rightOffset1 = 1;
   int rightOffset2 = 1;
   int frmIconHeight = m_pWinIcon->pixmap()->height();
   int frmIconWidth  = m_pWinIcon->pixmap()->width();
   int frmIconOffset = 1;
   QWidget* pIconWidget = m_pWinIcon;
   m_pCaption->setGeometry( KMDI_CHILDFRM_BORDER, KMDI_CHILDFRM_BORDER, captionWidth, captionHeight);
   //The buttons are caption children
   if (KMdiMainFrm::frameDecorOfAttachedViews() == KMdi::Win95Look) {
      rightOffset2 += 2;
      m_pUnixIcon->hide();
   }
   else if (KMdiMainFrm::frameDecorOfAttachedViews() == KMdi::KDE1Look) {
      buttonWidth  += 4;
      buttonHeight += 4;
      heightOffset -= 2;
      rightOffset1 = 0;
      rightOffset2 = 0;
      m_pWinIcon->hide();
      frmIconHeight = buttonHeight;
      frmIconWidth  = buttonWidth;
      frmIconOffset = 0;
      pIconWidget = m_pUnixIcon;
   }
   else if (KMdiMainFrm::frameDecorOfAttachedViews() == KMdi::KDELook) {
      buttonWidth  += 3;
      buttonHeight += 3;
      heightOffset -= 1;
      m_pUnixIcon->hide();
   }
   if (KMdiMainFrm::frameDecorOfAttachedViews() != KMdi::KDELaptopLook) {
      pIconWidget->setGeometry(frmIconOffset,captionHeight/2-frmIconHeight/2,frmIconWidth,frmIconHeight);
      m_pClose->setGeometry((captionWidth-buttonWidth)-rightOffset1,heightOffset,buttonWidth,buttonHeight);
      m_pMaximize->setGeometry((captionWidth-(buttonWidth*2))-rightOffset2,heightOffset,buttonWidth,buttonHeight);
      m_pMinimize->setGeometry((captionWidth-(buttonWidth*3))-rightOffset2,heightOffset,buttonWidth,buttonHeight);
      m_pUndock->setGeometry((captionWidth-(buttonWidth*4))-rightOffset2,heightOffset,buttonWidth,buttonHeight);
   }
   else {   // KDELaptopLook
      m_pWinIcon->hide();
      m_pUnixIcon->hide();
      buttonHeight += 5;
      heightOffset -= 2;
      m_pClose->setGeometry   ( 0,                 heightOffset, 27, buttonHeight);
      m_pMaximize->setGeometry( captionWidth-27,   heightOffset, 27, buttonHeight);
      m_pMinimize->setGeometry( captionWidth-27*2, heightOffset, 27, buttonHeight);
      m_pUndock->setGeometry  ( captionWidth-27*3, heightOffset, 27, buttonHeight);
   }
   //Resize the client
   if (!captionOnly && m_pClient) {
      QSize newClientSize(captionWidth,
      height()-(KMDI_CHILDFRM_DOUBLE_BORDER+captionHeight+KMDI_CHILDFRM_SEPARATOR));
      if (newClientSize != m_pClient->size()) {
         m_pClient->setGeometry(KMDI_CHILDFRM_BORDER,
                                m_pCaption->heightHint()+KMDI_CHILDFRM_SEPARATOR+KMDI_CHILDFRM_BORDER,
                                newClientSize.width(), newClientSize.height());
      }
   }
}

static bool hasParent(QObject* par, QObject* o) {
	while (o && o!=par)
		o = o->parent();
	return o==par;
}

//============= eventFilter ===============//

bool KMdiChildFrm::eventFilter( QObject *obj, QEvent *e )
{
   switch (e->type()) {
   case QEvent::Enter:
      {
         // check if the receiver is really a child of this frame
         bool bIsChild = false;
         QObject*    pObj = obj;
         while ( (pObj != 0L) && !bIsChild) {
            bIsChild = (pObj == this);
            pObj = pObj->parent();
         }
         // unset the resize cursor if the cursor moved from the frame into a inner widget
         if (bIsChild) {
            unsetResizeCursor();
         }
      }
      break;
   case QEvent::MouseButtonPress:
      {
//         if ( (QWidget*)obj != m_pClient ) {
         if (!hasParent(m_pClient, obj)) {
            bool bIsSecondClick = false;
            if (m_timeMeasure.elapsed() <= QApplication::doubleClickInterval()) {
               bIsSecondClick = true;  // of a possible double click
            }
            if ( !(((obj == m_pWinIcon) || (obj == m_pUnixIcon)) && bIsSecondClick) ) {
               // in case we didn't click on the icon button
               QFocusEvent* pFE = new QFocusEvent(QFocusEvent::FocusIn);
               QApplication::sendEvent(qApp->mainWidget(), pFE);
               if (m_pClient) {
                  m_pClient->updateTimeStamp();
                  m_pClient->activate();
               }
               QWidget* w = (QWidget*) obj;
               if( (w->parent() != m_pCaption) && (w != m_pCaption)) {
                  if ((w->focusPolicy() == Qt::ClickFocus) || (w->focusPolicy() == Qt::StrongFocus)) {
                     w->setFocus();
                  }
               }
            }
            if ((obj == m_pWinIcon) || (obj == m_pUnixIcon)) {
               // in case we clicked on the icon button
               if (m_timeMeasure.elapsed() > QApplication::doubleClickInterval()) {
                  showSystemMenu();
                  m_timeMeasure.start();
               }
               else {
                  closePressed();   // double click on icon button closes the view
               }
               return true;
            }
         }
      }
      break;
   case QEvent::Resize:
      {
         if ( ( (QWidget*)obj == m_pClient ) && (m_state == Normal) ) {
            QResizeEvent* re = (QResizeEvent*)e;
            int captionHeight = m_pCaption->heightHint();
            QSize newChildFrmSize( re->size().width() + KMDI_CHILDFRM_DOUBLE_BORDER,
               re->size().height() + captionHeight + KMDI_CHILDFRM_SEPARATOR + KMDI_CHILDFRM_DOUBLE_BORDER );
            if( newChildFrmSize != size())
               resize( newChildFrmSize );
         }
      }
      break;
   case QEvent::ChildRemoved:
      {
         // if we lost a child we uninstall ourself as event filter for the lost
         // child and its children
         QObject* pLostChild = ((QChildEvent*)e)->child();
         if ((pLostChild != 0L) /*&& (pLostChild->inherits("QWidget"))*/ ) {
            QObjectList *list = pLostChild->queryList();
            list->insert(0, pLostChild);        // add the lost child to the list too, just to save code
            QObjectListIt it( *list );          // iterate over all lost child widgets
            QObject * obj;
            while ( (obj=it.current()) != 0 ) { // for each found object...
               QWidget* widg = (QWidget*)obj;
               ++it;
               widg->removeEventFilter(this);
            }
            delete list;                        // delete the list, not the objects
         }
      }
      break;
   case QEvent::ChildAdded:
      {
         // if we got a new child we install ourself as event filter for the new
         // child and its children (as we did when we got our client).
         // XXX see linkChildren() and focus policy stuff
         QObject* pNewChild = ((QChildEvent*)e)->child();
         if ( (pNewChild != 0L) && (pNewChild->inherits("QWidget")) ) {
            QWidget* pNewWidget = (QWidget*)pNewChild;
            QObjectList *list = pNewWidget->queryList( "QWidget" );
            list->insert(0, pNewChild);         // add the new child to the list too, just to save code
            QObjectListIt it( *list );          // iterate over all new child widgets
            QObject * obj;
            while ( (obj=it.current()) != 0 ) { // for each found object...
               QWidget* widg = (QWidget*)obj;
               ++it;
               if (!(widg->inherits("QPopupMenu"))) {
                  widg->installEventFilter(this);
               }
            }
            delete list;                        // delete the list, not the objects
         }
      }
      break;
   default:
      break;
   }
   // return QWidget::eventFilter( obj, e);  // standard event processing
   return false;                  // standard event processing (see Qt documentation)
}

//============= raiseAndActivate ===============//

void KMdiChildFrm::raiseAndActivate()
{
   //qDebug("ChildFrm::raiseAndActivate");
   m_pCaption->setActive(true);
   m_pManager->setTopChild(this,false); //Do not focus by now...
}

//============= setMinimumSize ===============//

void KMdiChildFrm::setMinimumSize ( int minw, int minh )
{
   QWidget::setMinimumSize(minw, minh);
   if (m_state == Maximized) {
      m_pManager->setMinimumSize(minw, minh);
   }
}

//============= systemMenu ===============//

Q3PopupMenu* KMdiChildFrm::systemMenu() const
{
   if( m_pSystemMenu == 0)
      return 0;

   m_pSystemMenu->clear();

   if (KMdiMainFrm::frameDecorOfAttachedViews() != KMdi::Win95Look) {
      m_pSystemMenu->insertItem(i18n("&Restore"),this,SLOT(restorePressed()));
      m_pSystemMenu->insertItem(i18n("&Move"),m_pCaption, SLOT(slot_moveViaSystemMenu()));
      m_pSystemMenu->insertItem(i18n("R&esize"),this, SLOT(slot_resizeViaSystemMenu()));
      m_pSystemMenu->insertItem(i18n("M&inimize"),this, SLOT(minimizePressed()));
      m_pSystemMenu->insertItem(i18n("M&aximize"),this, SLOT(maximizePressed()));
      if( state() == Normal)
         m_pSystemMenu->setItemEnabled(m_pSystemMenu->idAt(0),false);
      else if( state() == Maximized) {
         m_pSystemMenu->setItemEnabled(m_pSystemMenu->idAt(1),false);
         m_pSystemMenu->setItemEnabled(m_pSystemMenu->idAt(2),false);
         m_pSystemMenu->setItemEnabled(m_pSystemMenu->idAt(4),false);
      }
      else if( state() == Qt::DockMinimized) {
         m_pSystemMenu->setItemEnabled(m_pSystemMenu->idAt(2),false);
         m_pSystemMenu->setItemEnabled(m_pSystemMenu->idAt(3),false);
      }
   }
   else  {
      if( state() != Normal)
         m_pSystemMenu->insertItem(i18n("&Restore"),this,SLOT(restorePressed()));
      if( state() != Maximized)
         m_pSystemMenu->insertItem(i18n("&Maximize"),this, SLOT(maximizePressed()));
      if( state() != Qt::DockMinimized)
         m_pSystemMenu->insertItem(i18n("&Minimize"),this, SLOT(minimizePressed()));
      if( state() != Maximized)
         m_pSystemMenu->insertItem(i18n("M&ove"),m_pCaption, SLOT(slot_moveViaSystemMenu()));
      if( state() == Normal)
         m_pSystemMenu->insertItem(i18n("&Resize"),this, SLOT(slot_resizeViaSystemMenu()));
   }

   m_pSystemMenu->insertItem(i18n("&Undock"),this, SLOT(undockPressed()));
   m_pSystemMenu->insertSeparator();
   m_pSystemMenu->insertItem(i18n("&Close"),this, SLOT(closePressed()));

   return m_pSystemMenu;
}

/** Shows a system menu for child frame windows. */
void KMdiChildFrm::showSystemMenu()
{
   if (KMdiMainFrm::frameDecorOfAttachedViews() != KMdi::Win95Look) {
      m_pUnixIcon->setDown( false);
   }
   QPoint popupmenuPosition;
   //qDebug("%d,%d,%d,%d,%d",m_pIcon->pos().x(),x(),m_pIcon->pos().y(),m_pIcon->height(),y());
   QRect iconGeom;
   if (KMdiMainFrm::frameDecorOfAttachedViews() == KMdi::Win95Look)
      iconGeom = m_pWinIcon->geometry();
   else
      iconGeom = m_pUnixIcon->geometry();
   popupmenuPosition = QPoint( iconGeom.x(),
                               iconGeom.y() + captionHeight() + KMDI_CHILDFRM_BORDER );
   systemMenu()->popup( mapToGlobal( popupmenuPosition));
}

void KMdiChildFrm::switchToMinimizeLayout()
{
   setMinimumWidth(KMDI_CHILDFRM_MIN_WIDTH);
   setFixedHeight(m_pCaption->height()+KMDI_CHILDFRM_DOUBLE_BORDER);

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

void KMdiChildFrm::slot_resizeViaSystemMenu()
{
   grabMouse();
   m_bResizing = true;
   m_iResizeCorner = KMDI_RESIZE_BOTTOMLEFT;
   setResizeCursor( m_iResizeCorner);
}

void KMdiChildFrm::redecorateButtons()
{
   if (m_pMinButtonPixmap)
      delete m_pMinButtonPixmap;
   if (m_pMaxButtonPixmap)
      delete m_pMaxButtonPixmap;
   if (m_pRestoreButtonPixmap)
      delete m_pRestoreButtonPixmap;
   if (m_pCloseButtonPixmap)
      delete m_pCloseButtonPixmap;
   if (m_pUndockButtonPixmap)
      delete m_pUndockButtonPixmap;

   if (KMdiMainFrm::frameDecorOfAttachedViews() == KMdi::Win95Look) {
      m_pMinButtonPixmap = new QPixmap( win_minbutton);
      m_pMaxButtonPixmap = new QPixmap( win_maxbutton);
      m_pRestoreButtonPixmap = new QPixmap( win_restorebutton);
      m_pCloseButtonPixmap = new QPixmap( win_closebutton);
      m_pUndockButtonPixmap = new QPixmap( win_undockbutton);
   }
   else if (KMdiMainFrm::frameDecorOfAttachedViews() == KMdi::KDE1Look) {
      m_pMinButtonPixmap = new QPixmap( kde_minbutton);
      m_pMaxButtonPixmap = new QPixmap( kde_maxbutton);
      m_pRestoreButtonPixmap = new QPixmap( kde_restorebutton);
      m_pCloseButtonPixmap = new QPixmap( kde_closebutton);
      m_pUndockButtonPixmap = new QPixmap( kde_undockbutton);
   }
   else if (KMdiMainFrm::frameDecorOfAttachedViews() == KMdi::KDELook) {
      m_pMinButtonPixmap = new QPixmap( kde2_minbutton);
      m_pMaxButtonPixmap = new QPixmap( kde2_maxbutton);
      m_pRestoreButtonPixmap = new QPixmap( kde2_restorebutton);
      m_pCloseButtonPixmap = new QPixmap( kde2_closebutton);
      m_pUndockButtonPixmap = new QPixmap( kde2_undockbutton);
   }
   else {   // kde2laptop look
      m_pMinButtonPixmap = new QPixmap( kde2laptop_minbutton);
      m_pMaxButtonPixmap = new QPixmap( kde2laptop_maxbutton);
      m_pRestoreButtonPixmap = new QPixmap( kde2laptop_restorebutton);
      m_pCloseButtonPixmap = new QPixmap( kde2laptop_closebutton);
      m_pUndockButtonPixmap = new QPixmap( kde2laptop_undockbutton);
   }

   m_pUnixIcon->setAutoRaise(true);
   if (KMdiMainFrm::frameDecorOfAttachedViews() == KMdi::KDE1Look) {
      m_pMinimize->setAutoRaise(true);
      m_pMaximize->setAutoRaise(true);
      m_pClose->setAutoRaise(true);
      m_pUndock->setAutoRaise(true);
   }
   else {
      m_pMinimize->setAutoRaise(false);
      m_pMaximize->setAutoRaise(false);
      m_pClose->setAutoRaise(false);
      m_pUndock->setAutoRaise(false);
   }

   if (m_pClient && m_pClient->icon()) {
      m_pWinIcon->setPixmap( *(m_pClient)->icon());
      m_pUnixIcon->setPixmap( *(m_pClient)->icon());
   }
   else {
      m_pWinIcon->setPixmap( *m_pIconButtonPixmap);
      m_pUnixIcon->setPixmap( *m_pIconButtonPixmap);
   }
   m_pClose->setPixmap( *m_pCloseButtonPixmap);
   m_pMinimize->setPixmap( *m_pMinButtonPixmap);
   m_pMaximize->setPixmap( *m_pMaxButtonPixmap);
   m_pUndock->setPixmap( *m_pUndockButtonPixmap);
}

QRect KMdiChildFrm::mdiAreaContentsRect() const
{
   Q3Frame* p = (Q3Frame*)parentWidget();
   if (p) {
      return p->contentsRect();
   }
   else {
      QRect empty;
      return empty;
   }
}

// kate: space-indent on; indent-width 2; replace-tabs on;
