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

#include "qextmditaskbar.h"
#include "qextmdimainfrm.h"
#include "qextmdichildview.h"
#include "qextmdidefines.h"

#include <qtooltip.h>

#if QT_VERSION >= 200
#include <qnamespace.h>
#endif

/*
	@quickhelp: QextMdiTaskBar
	@widget: Taskbar
		This window lists the currently open windows.<br>
		Each button corresponds to a single MDI (child) window.<br>
		The button is enabled (clickable) when the window is docked , and can be
		pressed to bring it to the top of the other docked windows.<br>
		The button text becomes red when new output is shown in the window and it is not the active one.<br>
*/

//####################################################################
//
// QextMdiTaskBarButton
//
//####################################################################

QextMdiTaskBarButton::QextMdiTaskBarButton(QextMdiTaskBar *pTaskBar,QextMdiChildView *win_ptr)
:QToolButton(pTaskBar)
{
   setToggleButton( true); //F.B.

//	setFont(g_pOptions->m_fntApplication);
	m_pWindow      = win_ptr;
	QToolTip::add(this,win_ptr->caption());
}

QextMdiTaskBarButton::~QextMdiTaskBarButton()
{
}

void QextMdiTaskBarButton::mousePressEvent( QMouseEvent* e)
{
   switch(e->button()) {
#if QT_VERSION >= 200
   case QMouseEvent::LeftButton:
#else
   case LeftButton:
#endif
      emit leftMouseButtonClicked( m_pWindow);
      break;
#if QT_VERSION >= 200
   case QMouseEvent::RightButton:
#else
   case RightButton:
#endif
      emit rightMouseButtonClicked( m_pWindow);
      break;
   default:
      break;
   }
   emit clicked( m_pWindow);
}

void QextMdiTaskBarButton::setNewText(const QString& s)
{
   setText(s);
}

//####################################################################
//
// QextMdiTaskBar
//
//####################################################################

QextMdiTaskBar::QextMdiTaskBar(QextMdiMainFrm *parent,QMainWindow::ToolBarDock dock)
:QToolBar("TaskBar",parent,dock)
{
	m_pFrm = parent;
	m_pButtonList = new QList<QextMdiTaskBarButton>;
	m_pButtonList->setAutoDelete(true);
//	setFont(g_pOptions->m_fntApplication);
	setFontPropagation(QWidget::SameFont);
	m_pCurrentFocusedWindow = 0;
}

QextMdiTaskBar::~QextMdiTaskBar()
{
	delete m_pButtonList;
}

QextMdiTaskBarButton * QextMdiTaskBar::addWinButton(QextMdiChildView *win_ptr)
{
	QextMdiTaskBarButton *b=new QextMdiTaskBarButton( this /*0,                      // icon
	                                    win_ptr->caption(),     // text
	                                    "",                     // statusbar text
	                                    this,                   // receiver
	                                    SLOT(setActiveButton()),// slot
	                                    this);                  // parent*/ ,win_ptr);
	
   connect( b, SIGNAL(clicked()), win_ptr, SLOT(setFocus()) );	
   connect( b, SIGNAL(clicked(QextMdiChildView*)), this, SLOT(setActiveButton(QextMdiChildView*)) );
   connect( b,  SIGNAL(leftMouseButtonClicked(QextMdiChildView*)), m_pFrm, SLOT(taskbarButtonLeftClicked(QextMdiChildView*)) );
   connect( b, SIGNAL(rightMouseButtonClicked(QextMdiChildView*)), m_pFrm, SLOT(taskbarButtonRightClicked(QextMdiChildView*)) );
	
	m_pButtonList->append(b);
	b->setToggleButton( true);
	b->setText(win_ptr->caption());   //F.B.
	b->show(); //????
	return b;
}

void QextMdiTaskBar::removeWinButton(QextMdiChildView *win_ptr)
{
	QextMdiTaskBarButton *b=getButton(win_ptr);
	if(b){
		m_pButtonList->removeRef(b);
	}
}
/*F.B.
void QextMdiTaskBar::windowAttached(QextMdiChildView *win_ptr,bool bAttached)
{
//F.B.	QToolButton *b=getButton(win_ptr);
//F.B.	b->setAttachState(bAttached);
}
F.B.*/
QextMdiTaskBarButton * QextMdiTaskBar::getButton(QextMdiChildView *win_ptr)
{
	for(QextMdiTaskBarButton *b=m_pButtonList->first();b;b=m_pButtonList->next()){
		if(b->m_pWindow == win_ptr)return b;
	}
	return 0;
}

QextMdiTaskBarButton * QextMdiTaskBar::getNextWindowButton(bool bRight,QextMdiChildView *win_ptr)
{
	if(bRight){
		for(QextMdiTaskBarButton *b=m_pButtonList->first();b;b=m_pButtonList->next()){
			if(b->m_pWindow == win_ptr){
				b = m_pButtonList->next();
				if(!b)b = m_pButtonList->first();
				if(win_ptr != b->m_pWindow)return b;
				else return 0;
			}
		}
	} else {
		for(QextMdiTaskBarButton *b=m_pButtonList->first();b;b=m_pButtonList->next()){
			if(b->m_pWindow == win_ptr){
				b = m_pButtonList->prev();
				if(!b)b = m_pButtonList->last();
				if(win_ptr != b->m_pWindow)return b;
				else return 0;
			}
		}
	}
	return 0;
}

void QextMdiTaskBar::setActiveButton(QextMdiChildView *win_ptr)
{
	QextMdiTaskBarButton* newPressedButton = 0;
	QextMdiTaskBarButton* oldPressedButton = 0;
	for(QextMdiTaskBarButton *b=m_pButtonList->first();b;b=m_pButtonList->next()){
	   if( b->m_pWindow == win_ptr)
	      newPressedButton = b;
	   if( b->m_pWindow == m_pCurrentFocusedWindow)
	      oldPressedButton = b;
	}
	
   if( newPressedButton != oldPressedButton) {
         if( oldPressedButton != 0)
            oldPressedButton->toggle();// switch off
   	   newPressedButton->toggle();   // switch on
   	   m_pCurrentFocusedWindow = win_ptr;
   }
}

/*F.B.void QextMdiTaskBar::fontChange(const QFont &oldFont)
{
	recalcButtonHeight();
}
F.B.*/







