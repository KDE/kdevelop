//----------------------------------------------------------------------------
//    $Id$
//----------------------------------------------------------------------------
//    Project              : Qt MDI extension
//
//    begin                : 07/1999       by Szymon Stefanek as part of kvirc
//                                         (an IRC application)
//    changes              : 09/1999       by Falk Brettschneider to create an
//                                         stand-alone Qt extension set of
//                                         classes and a Qt-based library
//
//    copyright            : (C) 1999-2000 by Szymon Stefanek (stefanek@tin.it)
//                                         and
//                                         Falk Brettschneider
//    email                :  gigafalk@geocities.com (Falk Brettschneider)
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

#include "qextmdidefines.h"
#include "qextmdichildfrmcaption.h"
#include "qextmdichildarea.h"
#include "qextmdichildfrm.h"

#include <qpainter.h>
#include <qapplication.h>
#include <qcursor.h>

#if QT_VERSION >= 200
#include <qnamespace.h>
#endif

#include <qobjectlist.h>

//////////////////////////////////////////////////////////////////////////////
// QextMdiChildFrm
//////////////////////////////////////////////////////////////////////////////

#define KVI_MDI_NORESIZE 0
#define KVI_MDI_RESIZE_TOP 1
#define KVI_MDI_RESIZE_LEFT 2
#define KVI_MDI_RESIZE_RIGHT 4
#define KVI_MDI_RESIZE_BOTTOM 8
#define KVI_MDI_RESIZE_TOPLEFT (1|2)
#define KVI_MDI_RESIZE_TOPRIGHT (1|4)
#define KVI_MDI_RESIZE_BOTTOMLEFT (8|2)
#define KVI_MDI_RESIZE_BOTTOMRIGHT (8|4)

#include "closebutton.xpm"
#include "minbutton.xpm"
#include "maxbutton.xpm"
#include "restorebutton.xpm"
#include "undockbutton.xpm"

//============ QextMdiChildFrm ============//

QextMdiChildFrm::QextMdiChildFrm(QextMdiChildArea *parent)
:QFrame(parent)
{
  m_pClient   = 0;
  m_pCaption  = new QextMdiChildFrmCaption(this);
  m_pManager  = parent;
  
  m_resizeMode = false;
  
  m_pMinimize = new QPushButton(m_pCaption);
  connect(m_pMinimize,SIGNAL(clicked()),this,SLOT(minimizePressed()));
  m_pMaximize = new QPushButton(m_pCaption);
  connect(m_pMaximize,SIGNAL(clicked()),this,SLOT(maximizePressed()));
  m_pClose    = new QPushButton(m_pCaption);
  connect(m_pClose,SIGNAL(clicked()),this,SLOT(closePressed()));
  m_pUndock   = new QPushButton(m_pCaption);
  connect(m_pUndock,SIGNAL(clicked()),this,SLOT(undockPressed()));
  
  m_pMinButtonPixmap = new QPixmap( minbutton);
  m_pMaxButtonPixmap = new QPixmap( maxbutton);
  m_pRestoreButtonPixmap = new QPixmap( restorebutton);
  m_pCloseButtonPixmap = new QPixmap( closebutton);
  m_pUndockButtonPixmap = new QPixmap( undockbutton);
  
   m_pClose->setPixmap( *m_pCloseButtonPixmap);
   m_pClose->setFocusProxy( m_pCaption);
   m_pMinimize->setPixmap( *m_pMinButtonPixmap);
   m_pMinimize->setFocusProxy( m_pCaption);
   m_pMaximize->setPixmap( *m_pMaxButtonPixmap);
   m_pMaximize->setFocusProxy( m_pCaption);
   m_pUndock->setPixmap( *m_pUndockButtonPixmap);
   m_pUndock->setFocusProxy( m_pCaption);
   
   setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   //	setFocusPolicy(ClickFocus);

   m_state=Normal;
   
   m_iResizeCorner=KVI_MDI_NORESIZE;
   m_iLastCursorCorner=KVI_MDI_NORESIZE;
   setMouseTracking(true);
   setMinimumSize(QSize(QEXTMDI_MDI_CHILDFRM_MIN_WIDTH,QEXTMDI_MDI_CHILDFRM_MIN_HEIGHT));
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
	m_pCaption->setActive(true);
   m_pManager->setTopChild(this,false);

	m_iResizeCorner=getResizeCorner(e->pos().x(),e->pos().y());
	if(m_iResizeCorner != KVI_MDI_NORESIZE) {
	   m_resizeMode = true; //F.B.
	}
}

//============ mouseReleaseEvent ==============//

void QextMdiChildFrm::mouseReleaseEvent(QMouseEvent *)
{
	if(QApplication::overrideCursor())QApplication::restoreOverrideCursor();
	m_resizeMode = false;   //F.B.
}

//============= setResizeCursor ===============//

void QextMdiChildFrm::setResizeCursor(int resizeCorner)
{
	if(resizeCorner == m_iLastCursorCorner)return; //Don't do it twice
	m_iLastCursorCorner = resizeCorner;
	switch (resizeCorner) {
		case KVI_MDI_NORESIZE:
			if(QApplication::overrideCursor())QApplication::restoreOverrideCursor();
			break;
		case KVI_MDI_RESIZE_LEFT:
		case KVI_MDI_RESIZE_RIGHT:
#if QT_VERSION >= 200
			QApplication::setOverrideCursor(Qt::sizeHorCursor,true);
#else
			QApplication::setOverrideCursor(sizeHorCursor,true);
#endif
			break;
		case KVI_MDI_RESIZE_TOP:
		case KVI_MDI_RESIZE_BOTTOM:
#if QT_VERSION >= 200
			QApplication::setOverrideCursor(Qt::sizeVerCursor,true);
#else
			QApplication::setOverrideCursor(sizeVerCursor,true);
#endif
			break;
		case KVI_MDI_RESIZE_TOPLEFT:
		case KVI_MDI_RESIZE_BOTTOMRIGHT:
#if QT_VERSION >= 200
			QApplication::setOverrideCursor(Qt::sizeFDiagCursor,true);
#else
			QApplication::setOverrideCursor(sizeFDiagCursor,true);
#endif
			break;
		case KVI_MDI_RESIZE_BOTTOMLEFT:
		case KVI_MDI_RESIZE_TOPRIGHT:
#if QT_VERSION >= 200
			QApplication::setOverrideCursor(Qt::sizeBDiagCursor,true);
#else
			QApplication::setOverrideCursor(sizeBDiagCursor,true);
#endif
			break;
	}
}
#include <iostream.h>
//============= mouseMoveEvent ===============//

void QextMdiChildFrm::mouseMoveEvent(QMouseEvent *e)
{
	if(e->state() & LeftButton){
		if(m_iResizeCorner)
		   if(m_resizeMode) {
		      QPoint mousePos( e->pos().x(), e->pos().y());   //F.B.
		      resizeWindow(m_iResizeCorner, mousePos.x(), mousePos.y());
         }
	}
	else {
	   m_iResizeCorner = getResizeCorner(e->pos().x(), e->pos().y());
   	setResizeCursor( m_iResizeCorner);
   }
}

//=============== leaveEvent ===============//

void QextMdiChildFrm::leaveEvent(QEvent *)
{
   if(!m_resizeMode) {
	   m_iResizeCorner=KVI_MDI_NORESIZE;
   	m_iLastCursorCorner=KVI_MDI_NORESIZE;
      if(QApplication::overrideCursor())QApplication::restoreOverrideCursor();
   }
}

void QextMdiChildFrm::resizeWindow(int resizeCorner/*F.B.*/,int xPos, int yPos /*F.B.*/)
{
	QRect resizeRect(x(),y(),width(),height());

	// Calculate the minimum width & height
	int minWidth=0;
	int minHeight=0;
	if(m_pClient){
		minWidth  = m_pClient->minimumSize().width() + QEXTMDI_MDI_CHILDFRM_DOUBLE_BORDER;
		minHeight = m_pClient->minimumSize().height()+ QEXTMDI_MDI_CHILDFRM_DOUBLE_BORDER +
					m_pCaption->heightHint() + QEXTMDI_MDI_CHILDFRM_SEPARATOR;
	}
	if(minWidth<QEXTMDI_MDI_CHILDFRM_MIN_WIDTH)minWidth=QEXTMDI_MDI_CHILDFRM_MIN_WIDTH;
	if(minHeight<QEXTMDI_MDI_CHILDFRM_MIN_WIDTH)minHeight=QEXTMDI_MDI_CHILDFRM_MIN_HEIGHT;

	QPoint mousePos(xPos+x(),yPos+y());
	
	switch (resizeCorner){
	case KVI_MDI_RESIZE_LEFT:
		resizeRect.setLeft(mousePos.x());
		if(resizeRect.width() < minWidth)resizeRect.setLeft(resizeRect.right() - minWidth);
		break;
	case KVI_MDI_RESIZE_RIGHT:
		resizeRect.setRight(mousePos.x());
		if(resizeRect.width() < minWidth)resizeRect.setRight(resizeRect.left() + minWidth);
		break;
	case KVI_MDI_RESIZE_TOP:
		resizeRect.setTop(mousePos.y());
		if(resizeRect.height() < minHeight)resizeRect.setTop(resizeRect.bottom() - minHeight);
		break;
	case KVI_MDI_RESIZE_BOTTOM:
		resizeRect.setBottom(mousePos.y());
		if(resizeRect.height() < minHeight)resizeRect.setBottom(resizeRect.top() + minHeight);
		break;
	case KVI_MDI_RESIZE_BOTTOMRIGHT:
		resizeRect.setBottom(mousePos.y());
		if(resizeRect.height() < minHeight)resizeRect.setBottom(resizeRect.top() + minHeight);
		resizeRect.setRight(mousePos.x());
		if(resizeRect.width() < minWidth)resizeRect.setRight(resizeRect.left() + minWidth);
		break;
	case KVI_MDI_RESIZE_TOPRIGHT:
		resizeRect.setTop(mousePos.y());
		if(resizeRect.height() < minHeight)resizeRect.setTop(resizeRect.bottom() - minHeight);
		resizeRect.setRight(mousePos.x());
		if(resizeRect.width() < minWidth)resizeRect.setRight(resizeRect.left() + minWidth);
		break;
	case KVI_MDI_RESIZE_BOTTOMLEFT:
		resizeRect.setBottom(mousePos.y());
		if(resizeRect.height() < minHeight)resizeRect.setBottom(resizeRect.top() + minHeight);
		resizeRect.setLeft(mousePos.x());			
		if(resizeRect.width() < minWidth)resizeRect.setLeft(resizeRect.right() - minWidth);
		break;
	case KVI_MDI_RESIZE_TOPLEFT:
		resizeRect.setTop(mousePos.y());
		if(resizeRect.height() < minHeight)resizeRect.setTop(resizeRect.bottom() - minHeight);
		resizeRect.setLeft(mousePos.x());
		if(resizeRect.width() < minWidth)resizeRect.setLeft(resizeRect.right() - minWidth);
		break;
	}

	setGeometry(resizeRect.x(),resizeRect.y(),resizeRect.width(),resizeRect.height());

	if(m_state==Maximized){
		m_state=Normal;
		m_pMaximize->setPixmap( *m_pMaxButtonPixmap);
	}
}

//=========== moveWindow ==============//

void QextMdiChildFrm::moveWindow(/*F.B.*/QPoint diff /*F.B.*/)
{

	if(m_state==Maximized){
		m_state=Normal;
		m_pMaximize->setPixmap( *m_pMaxButtonPixmap);
	}
	setGeometry( x()+diff.x(), y()+diff.y(), width(), height());
}

//================= getResizeCorner =============//

int QextMdiChildFrm::getResizeCorner(int ax,int ay)
{
	int ret = KVI_MDI_NORESIZE;
	if((ax>0)&&(ax<(QEXTMDI_MDI_CHILDFRM_BORDER+2))) ret |= KVI_MDI_RESIZE_LEFT;
	if((ax<width())&&(ax>(width()-(QEXTMDI_MDI_CHILDFRM_BORDER+2)))) ret |= KVI_MDI_RESIZE_RIGHT;
	if((ay>0)&&(ay<(QEXTMDI_MDI_CHILDFRM_BORDER+2))) ret |= KVI_MDI_RESIZE_TOP;
	if((ay<(height()))&&(ay>(height()-(QEXTMDI_MDI_CHILDFRM_BORDER+2)))) ret |= KVI_MDI_RESIZE_BOTTOM;
	return ret;
}

//============= maximizePressed ============//

void QextMdiChildFrm::maximizePressed()
{
	switch(m_state){
		case Maximized: setState(Normal);    break;
		case Normal:    setState(Maximized); break;
		case Minimized: setState(Maximized); break;
	}
}

//============= minimizePressed ============//

void QextMdiChildFrm::minimizePressed()
{
	switch(m_state){
		case Minimized: setState(Normal);    break;
		case Normal:    setState(Minimized); break;
		case Maximized: setState(Minimized); break;
	}
}

//============= closePressed ============//

void QextMdiChildFrm::closePressed()
{
	if(m_pClient)m_pClient->close();
}

//============= undockPressed ============//

void QextMdiChildFrm::undockPressed()
{
	if(m_pClient) m_pManager->undockWindow(m_pClient);
}

//============ setState =================//

void QextMdiChildFrm::setState(MdiWindowState state,bool bAnimate)
{
//	debug("set state %d ",state);
	if(m_state==Normal){ //save the current rect
		m_restoredRect=QRect(x(),y(),width(),height());
	}
	QRect begin(x(),y(),width(),height());
	QRect end=begin;
	switch(state){
		case Normal:
			switch(m_state){
				case Maximized:
					//F.B. if(bAnimate)m_pManager->animate(begin,m_restoredRect);
					m_pMaximize->setPixmap( *m_pMaxButtonPixmap);
					setGeometry(m_restoredRect);
					m_state=state;
					break;
				case Minimized:
					begin=QRect(x()+width()/2,y()+height()/2,1,1);
					//F.B. if(bAnimate)m_pManager->animate(begin,end);
					m_pMaximize->setPixmap( *m_pMaxButtonPixmap);
					show();
					m_state=state;
					break;
				case Normal:
					break;
			}
			break;
		case Maximized:
			end=QRect(0,0,m_pManager->width(),m_pManager->height());
			switch(m_state){
				case Minimized:
					m_state=state;
					begin=QRect(x()+width()/2,y()+height()/2,1,1);
					//F.B. if(bAnimate)m_pManager->animate(begin,end);
					setGeometry(m_pManager->rect());
					m_pMaximize->setPixmap( *m_pRestoreButtonPixmap);
					show();
					break;
				case Normal:
//					debug("Yeppp...");
					m_state=state;
					//F.B. if(bAnimate)m_pManager->animate(begin,end);
					m_pMaximize->setPixmap( *m_pRestoreButtonPixmap);
					setGeometry(m_pManager->rect());
					break;
				case Maximized:
   				break;
			}
			break;
		case Minimized:
			end=QRect(x()+width()/2,y()+height()/2,1,1);
			switch(m_state){
				case Maximized:
					hide();
					//F.B. if(bAnimate)m_pManager->animate(begin,end);
					setGeometry(m_restoredRect);
					m_state=state;
					m_pManager->childMinimized(this,true);
					break;
				case Normal:
					hide();
					//F.B. if(bAnimate)m_pManager->animate(begin,end);
					m_state=state;
					m_pManager->childMinimized(this,false);
					break;
				case Minimized:
					break;
			}
			break;
	}
	bAnimate = false; //dummy, only to avoid "unused parameter"
}

//============ setCaption ===============//

void QextMdiChildFrm::setCaption(const QString& text)
{
	m_pCaption->setCaption(text);
	m_pManager->fillWindowMenu();
}

//============ enableClose ==============//

void QextMdiChildFrm::enableClose(bool bEnable)
{
	m_pClose->setEnabled(bEnable);
	m_pClose->repaint(false);
}

//============ setIconPointer =============//

void QextMdiChildFrm::setIconPointer(QPixmap *ptr)
{
	m_pCaption->m_pIcon=ptr;
	m_pCaption->repaint(false);
}

//============ setClient ============//

void QextMdiChildFrm::setClient(QextMdiChildView *w)
{
	m_pClient=w;
	//resize to match the client
	int clientYPos=m_pCaption->heightHint()+QEXTMDI_MDI_CHILDFRM_SEPARATOR+QEXTMDI_MDI_CHILDFRM_BORDER;
	resize(w->width()+QEXTMDI_MDI_CHILDFRM_DOUBLE_BORDER,w->height()+QEXTMDI_MDI_CHILDFRM_BORDER+clientYPos);
	//Reparent if needed
	if(w->parent()!=this){
		//reparent to this widget , no flags , point , show it
		QPoint pnt2(QEXTMDI_MDI_CHILDFRM_BORDER,clientYPos);
#if QT_VERSION >= 200
  	w->reparent(this,0,pnt2,true); //Flags get lost thru recreate! (DND too!)
#else
		w->recreate(this,0,pnt2,true); //Flags get lost thru recreate! (DND too!)
#endif
	} else w->move(QEXTMDI_MDI_CHILDFRM_BORDER,clientYPos);
	linkChildren();
	if(m_pClient->minimumSize().width() > QEXTMDI_MDI_CHILDFRM_MIN_WIDTH &&
		m_pClient->minimumSize().height() > QEXTMDI_MDI_CHILDFRM_MIN_HEIGHT){
		setMinimumWidth(m_pClient->minimumSize().width() + QEXTMDI_MDI_CHILDFRM_DOUBLE_BORDER);
		setMinimumHeight(m_pClient->minimumSize().height()+ QEXTMDI_MDI_CHILDFRM_DOUBLE_BORDER +
					m_pCaption->heightHint() + QEXTMDI_MDI_CHILDFRM_SEPARATOR);
	}
}

//============ unsetClient ============//

void QextMdiChildFrm::unsetClient()
{
	if(!m_pClient)return;
	//reparent to desktop widget , no flags , point , show it
	unlinkChildren();
	//Kewl...the reparent function has a small prob now..
	//the new toplelvel widgets gets not reenabled for dnd
#if QT_VERSION >= 200
	m_pClient->reparent(0,0,QPoint(0,0),true);
#else
	m_pClient->recreate(0,0,QPoint(0,0),true);
#endif
	m_pClient=0;
}

//============== linkChildren =============//
void QextMdiChildFrm::linkChildren()
{
   QObjectList *list = queryList( "QWidget" );
   QObjectListIt it( *list );          // iterate over the buttons
   QObject * obj;
   while ( (obj=it.current()) != 0 ) { // for each found object...
      ++it;
      ((QWidget*)obj)->setFocusPolicy(QWidget::StrongFocus);
      //((QWidget*)obj)->setFocusProxy(this);
      ((QWidget*)obj)->installEventFilter(this);
      int i;
      i = 1;
   }
   delete list;                        // delete the list, not the objects
}

//============== unlinkChildren =============//

void QextMdiChildFrm::unlinkChildren()
{
   QObjectList *list = queryList( "QWidget" );
   QObjectListIt it( *list );          // iterate over the buttons
   QObject * obj;
   while ( (obj=it.current()) != 0 ) { // for each found object...
      ++it;
      ((QWidget*)obj)->setFocusPolicy(QWidget::StrongFocus);
      //((QWidget*)obj)->setFocusProxy(0);
      ((QWidget*)obj)->removeEventFilter(this);
      int i;
      i = 1;
   }
   delete list;                        // delete the list, not the objects
}

//============== resizeEvent ===============//

void QextMdiChildFrm::resizeEvent(QResizeEvent *)
{
	//Resize the caption
	int captionHeight=m_pCaption->heightHint();
	int captionWidth=width()-QEXTMDI_MDI_CHILDFRM_DOUBLE_BORDER;
	m_pCaption->setGeometry(
		QEXTMDI_MDI_CHILDFRM_BORDER,
		QEXTMDI_MDI_CHILDFRM_BORDER,
		captionWidth,
		captionHeight
	);
	//The buttons are caption children
	m_pClose->setGeometry((captionWidth-captionHeight)+1,1,captionHeight-2,captionHeight-2);
	m_pMaximize->setGeometry((captionWidth-(captionHeight*2))+2,1,captionHeight-2,captionHeight-2);
	m_pMinimize->setGeometry((captionWidth-(captionHeight*3))+3,1,captionHeight-2,captionHeight-2);
	m_pUndock->setGeometry((captionWidth-(captionHeight*4))+4,1,captionHeight-2,captionHeight-2);   //F.B.
	//Resize the client
	if(m_pClient){
		m_pClient->setGeometry(
			QEXTMDI_MDI_CHILDFRM_BORDER,
			QEXTMDI_MDI_CHILDFRM_BORDER+captionHeight+QEXTMDI_MDI_CHILDFRM_SEPARATOR,
			captionWidth,
			height()-(QEXTMDI_MDI_CHILDFRM_DOUBLE_BORDER+captionHeight+QEXTMDI_MDI_CHILDFRM_SEPARATOR)
		);
	}
}

//============= focusInEvent ===============//

bool QextMdiChildFrm::eventFilter( QObject *, QEvent *e )
{
#if QT_VERSION >= 200
   if ( e->type() == QEvent::FocusIn ) {  // key press
#else
   if ( e->type() == Event_FocusIn ) {  // key press
#endif
      m_pCaption->setActive(true);
      m_pManager->setTopChild(this,false); //Do not focus by now...
   }
   return false;                           // standard event processing

}
