//----------------------------------------------------------------------------
//    filename             : qextmditaskbar.cpp
//----------------------------------------------------------------------------
//    Project              : Qt MDI extension
//
//    begin                : 07/1999       by Szymon Stefanek as part of kvirc
//                                         (an IRC application)
//    changes              : 09/1999       by Falk Brettschneider to create an
//                                         stand-alone Qt extension set of
//                                         classes and a Qt-based library
//                           02/2000       by Massimo Morin (mmorin@schedsys.com)
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

#include "qextmditaskbar.h"
#include "qextmdimainfrm.h"
#include "qextmdichildview.h"
#include "qextmdidefines.h"

#include <qtooltip.h>
#include <qlabel.h>

#include <qnamespace.h>

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
:QPushButton(pTaskBar),
 m_actualText("")
{
   setToggleButton( true);
	m_pWindow      = win_ptr;
	QToolTip::add(this,win_ptr->caption());
	setFocusPolicy(NoFocus);
}

QextMdiTaskBarButton::~QextMdiTaskBarButton()
{
}

void QextMdiTaskBarButton::mousePressEvent( QMouseEvent* e)
{
   switch(e->button()) {
   case QMouseEvent::LeftButton:
      emit leftMouseButtonClicked( m_pWindow);
      break;
   case QMouseEvent::RightButton:
      emit rightMouseButtonClicked( m_pWindow);
      break;
   default:
      break;
   }
   emit clicked( m_pWindow);
}

/** slot version of setText */
void QextMdiTaskBarButton::setNewText(const QString& s)
{
   setText( s);
}

void QextMdiTaskBarButton::setText(const QString& s)
{
   m_actualText = s;
   QButton::setText( s);
}

void QextMdiTaskBarButton::fitText(const QString& s, int newWidth)
{
   QButton::setText( m_actualText);

   int actualWidth = sizeHint().width();
   int realLetterCount = s.length();
   int newLetterCount = (newWidth * realLetterCount) / actualWidth;
   if( newLetterCount < realLetterCount) {
      if(newLetterCount > 3)
         QButton::setText( s.left( newLetterCount/2) + "..." + s.right( newLetterCount/2));
      else {
         if(newLetterCount > 1) QButton::setText( s.left( newLetterCount) + "..");
         else QButton::setText( s.left(1));
      }
   }
   else
      QButton::setText( s);
}

QString QextMdiTaskBarButton::actualText() const
{
   return m_actualText;
}

//####################################################################
//
// QextMdiTaskBar
//
//####################################################################

QextMdiTaskBar::QextMdiTaskBar(QextMdiMainFrm *parent,QMainWindow::ToolBarDock dock)
 :QToolBar("TaskBar",parent,dock),
  m_pCurrentFocusedWindow(0),
  m_pStretchSpace(0)
{
	m_pFrm = parent;
	m_pButtonList = new QList<QextMdiTaskBarButton>;
	m_pButtonList->setAutoDelete(true);
	setFontPropagation(QWidget::SameFont);
   setMinimumWidth(1);
   setFocusPolicy(NoFocus);
}

QextMdiTaskBar::~QextMdiTaskBar()
{
	delete m_pButtonList;
}

QextMdiTaskBarButton * QextMdiTaskBar::addWinButton(QextMdiChildView *win_ptr)
{
   if( m_pStretchSpace) {
      delete m_pStretchSpace;
      m_pStretchSpace = 0;
      setStretchableWidget( 0);
   }

	QextMdiTaskBarButton *b=new QextMdiTaskBarButton( this, win_ptr);
	
   QObject::connect( b, SIGNAL(clicked()), win_ptr, SLOT(setFocus()) );	
   QObject::connect( b, SIGNAL(clicked(QextMdiChildView*)), this, SLOT(setActiveButton(QextMdiChildView*)) );
   QObject::connect( b, SIGNAL(leftMouseButtonClicked(QextMdiChildView*)), m_pFrm, SLOT(activateView(QextMdiChildView*)) );
   QObject::connect( b, SIGNAL(rightMouseButtonClicked(QextMdiChildView*)), m_pFrm, SLOT(taskbarButtonRightClicked(QextMdiChildView*)) );
	
	m_pButtonList->append(b);
	b->setToggleButton( true);
	b->setText(win_ptr->tabCaption());
	
   m_pStretchSpace = new QLabel(this, "empty");
   m_pStretchSpace->setText("");
   setStretchableWidget( m_pStretchSpace);
   m_pStretchSpace->show();

	layoutTaskBar();
	b->show();
	return b;
}

void QextMdiTaskBar::removeWinButton(QextMdiChildView *win_ptr, bool haveToLayoutTaskBar)
{
	QextMdiTaskBarButton *b=getButton(win_ptr);
	if(b){
		m_pButtonList->removeRef(b);
      if( haveToLayoutTaskBar) layoutTaskBar();
	}
}

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

void QextMdiTaskBar::layoutTaskBar( int taskBarWidth)
{
   if( !taskBarWidth)
      // no width is given
      taskBarWidth = width();

   // calculate current width of all taskbar buttons
   int allButtonsWidth = 0;
   QextMdiTaskBarButton *b = 0;
	for(b=m_pButtonList->first();b;b=m_pButtonList->next()){
	   allButtonsWidth += b->width();
	}
	
	// reset button text
	for(b=m_pButtonList->first();b;b=m_pButtonList->next()){
	   b->setText( b->actualText());
	}

   // calculate actual width of all taskbar buttons
   int allButtonsWidthHint = 0;
	for(b=m_pButtonList->first();b;b=m_pButtonList->next()){
	   int shw = b->sizeHint().width();
	   allButtonsWidthHint += shw;
	} 

   // if there's enough space, use actual width
   if( allButtonsWidthHint <= taskBarWidth - 10) {
   	for(b=m_pButtonList->first();b;b=m_pButtonList->next()){
	      b->setMinimumWidth( b->sizeHint().width());
	      b->show();
	   }
   }
   else {
      // too many buttons for actual width
   	//if( allButtonsWidth > taskBarWidth - 10) {
         int buttonCount = m_pButtonList->count();
         int newButtonWidth = (taskBarWidth - 10) / buttonCount;
      	for(b=m_pButtonList->first();b;b=m_pButtonList->next()){
   	      b->fitText( b->actualText(), newButtonWidth);
	         b->setMinimumWidth( newButtonWidth);
	         b->show();
   	   }
   }
}

void QextMdiTaskBar::resizeEvent( QResizeEvent* rse)
{
   layoutTaskBar( rse->size().width());
   QToolBar::resize( rse->size());
}
