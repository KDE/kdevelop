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

#include "qextmdichildview.h"
#include "qextmdimainfrm.h"
#include "qextmdichildfrm.h"
#include "qextmdidefines.h"   //F.B.

#include <qdatetime.h>
//============ QextMdiChildView ============//

QextMdiChildView::QextMdiChildView( const QString& name, QWidget* parentWidget)
:QWidget(parentWidget)
{
   if( name)
      m_szCaption = name;
   else
      m_szCaption = QString("Unnamed");
   //F.B.	setMinimumSize(QSize(QEXTMDI_CHILDVIEW_MIN_WIDTH,QEXTMDI_CHILDVIEW_MIN_HEIGHT));
   setFocusPolicy(StrongFocus);
}

//============ ~QextMdiChildView ============//

QextMdiChildView::~QextMdiChildView()
{
}

QRect QextMdiChildView::externalGeometry()
{
	return mdiParent() ? mdiParent()->geometry() : geometry();
}

//============== minimize ==============//

void QextMdiChildView::minimize(bool bAnimate)
{
	if(mdiParent()){
		if(!isMinimized()){
			mdiParent()->setState(QextMdiChildFrm::Minimized,bAnimate);
		}
#if QT_VERSION >= 200
	} else showMinimized();
#else
	} else hide();
#endif
}

void QextMdiChildView::minimize(){ minimize(true); }

//============= maximize ==============//

void QextMdiChildView::maximize(bool bAnimate)
{
	if(mdiParent()){
		if(!isMaximized()){
			mdiParent()->setState(QextMdiChildFrm::Maximized,bAnimate);
		}
#if QT_VERSION >= 200
	} else showMaximized();
#else
  } else resize(QApplication::desktop()->size());
#endif
}

void QextMdiChildView::maximize(){ maximize(true); }

//============== attach ================//

void QextMdiChildView::attach()
{
   emit attachWindow(this,true,true,0);
}

//============== detach =================//

void QextMdiChildView::detach()
{
   emit detachWindow(this);
}

//=============== isMinimized ? =================//

bool QextMdiChildView::isMinimized()
{
	if(mdiParent())return (mdiParent()->state() == QextMdiChildFrm::Minimized);
	else return isMinimized();
}

//============== isMaximized ? ==================//

bool QextMdiChildView::isMaximized()
{
	if(mdiParent())return (mdiParent()->state() == QextMdiChildFrm::Maximized);
	if( size() == maximumSize()) return true; //F.B.
	else return false;//F.B.
}

//============== restore ================//

void QextMdiChildView::restore()
{
	if(mdiParent()){
		if(isMinimized()||isMaximized())mdiParent()->setState(QextMdiChildFrm::Normal);	
#if QT_VERSION >= 200
	} else showNormal();
#else
	} else resize(QApplication::desktop()->size());
#endif	
}

//=============== youAreAttached ============//

void QextMdiChildView::youAreAttached(QextMdiChildFrm *lpC)
{
	lpC->setCaption(m_szCaption);
	//F.B. lpC->setIconPointer(myIconPtr()); //It is ok to set a NULL icon too here
}

//================ youAreDetached =============//

void QextMdiChildView::youAreDetached()
{
	setCaption(m_szCaption);
	if(myIconPtr())setIcon(*(myIconPtr()));
	setFocusPolicy(QWidget::StrongFocus);
}

//================ setWindowCaption ================//

void QextMdiChildView::setWindowCaption(const QString& szCaption)
{
	m_szCaption=szCaption;
	if(mdiParent())mdiParent()->setCaption(m_szCaption);
	else setCaption(m_szCaption);
	emit windowCaptionChanged(m_szCaption);
}

//============== closeEvent ================//

void QextMdiChildView::closeEvent(QCloseEvent *e)
{
	e->ignore(); //we ignore the event , and then close later if needed.
	emit childWindowCloseRequest(this);
}

//================ myIconPtr =================//

QPixmap * QextMdiChildView::myIconPtr()
{
	return 0;
}
/*F.B.
//================ applyOptions =================//

void QextMdiChildView::applyOptions()
{
	//Nothing here
}

//=============== highlight =================//

void QextMdiChildView::highlight()
{
//F.B.	if(m_pTaskBarButton)m_pTaskBarButton->highlight();
}

//============ setProgress ==============//

void QextMdiChildView::setProgress(int progress)
{
//F.B.	if(m_pTaskBarButton)m_pTaskBarButton->setProgress(progress);
}
void QextMdiChildView::setProperties(QextMdiChildViewProperty *)
{
}

void QextMdiChildView::saveProperties()
{
}
F.B. */

//============= focusInEvent ===============//

void QextMdiChildView::focusInEvent(QFocusEvent *)
{
   emit focusInEventOccurs( this);
}


