//----------------------------------------------------------------------------
//    filename             : kmditaskbar.cpp
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

#include "kmditaskbar.h"
#include "kmditaskbar.moc"

#include "kmdimainfrm.h"
#include "kmdichildview.h"
#include "kmdidefines.h"

#include <qtooltip.h>
#include <qlabel.h>
#include <qwidget.h>
#include <qstyle.h>

#include <qnamespace.h>

/*
   @quickhelp: KMdiTaskBar
   @widget: Taskbar
      This window lists the currently open windows.<br>
      Each button corresponds to a single MDI (child) window.<br>
      The button is enabled (clickable) when the window is docked , and can be
      pressed to bring it to the top of the other docked windows.<br>
      The button text becomes red when new output is shown in the window and it is not the active one.<br>
*/

//####################################################################
//
// KMdiTaskBarButton
//
//####################################################################
KMdiTaskBarButton::KMdiTaskBarButton(KMdiTaskBar *pTaskBar,KMdiChildView *win_ptr)
:QPushButton(pTaskBar),
 m_actualText("")
{
   setToggleButton( true);
   m_pWindow      = win_ptr;
   QToolTip::add(this,win_ptr->caption());
   setFocusPolicy(NoFocus);
}

KMdiTaskBarButton::~KMdiTaskBarButton()
{
}

void KMdiTaskBarButton::mousePressEvent( QMouseEvent* e)
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
void KMdiTaskBarButton::setNewText(const QString& s)
{
   setText( s);
   emit buttonTextChanged( 0);
}

void KMdiTaskBarButton::setText(const QString& s)
{
   m_actualText = s;
   QButton::setText( s);
}

void KMdiTaskBarButton::fitText(const QString& origStr, int newWidth)
{
   QButton::setText( m_actualText);

   int actualWidth = sizeHint().width();
   int realLetterCount = origStr.length();
   int newLetterCount = (newWidth * realLetterCount) / actualWidth;
   int w = newWidth+1;
   QString s = origStr;
   while((w > newWidth) && (newLetterCount >= 1)) {
      if( newLetterCount < realLetterCount) {
         if(newLetterCount > 3)
            s = origStr.left( newLetterCount/2) + "..." + origStr.right( newLetterCount/2);
         else {
            if(newLetterCount > 1)
               s = origStr.left( newLetterCount) + "..";
            else
               s = origStr.left(1);
         }
      }
      QFontMetrics fm = fontMetrics();
      w = fm.width(s);
      newLetterCount--;
   }

   QButton::setText( s);
}

QString KMdiTaskBarButton::actualText() const
{
   return m_actualText;
}

//####################################################################
//
// KMdiTaskBar
//
//####################################################################

KMdiTaskBar::KMdiTaskBar(KMdiMainFrm *parent,QMainWindow::ToolBarDock dock)
:  KToolBar( parent, "KMdiTaskBar", /*honor_style*/ false, /*readConfig*/ true)
   ,m_pCurrentFocusedWindow(0)
   ,m_pStretchSpace(0)
   ,m_layoutIsPending(false)
   ,m_bSwitchedOn(false)
{
   m_pFrm = parent;
   m_pButtonList = new QPtrList<KMdiTaskBarButton>;
   m_pButtonList->setAutoDelete(true);
//QT30   setFontPropagation(QWidget::SameFont);
   setMinimumWidth(1);
   setFocusPolicy(NoFocus);
   parent->moveToolBar( this, dock); //XXX obsolete!
}

KMdiTaskBar::~KMdiTaskBar()
{
   delete m_pButtonList;
}

KMdiTaskBarButton * KMdiTaskBar::addWinButton(KMdiChildView *win_ptr)
{
   if( m_pStretchSpace) {
      delete m_pStretchSpace;
      m_pStretchSpace = 0L;
      setStretchableWidget( 0L);
   }

   KMdiTaskBarButton *b=new KMdiTaskBarButton( this, win_ptr);
   QObject::connect( b, SIGNAL(clicked()), win_ptr, SLOT(setFocus()) );
   QObject::connect( b, SIGNAL(clicked(KMdiChildView*)), this, SLOT(setActiveButton(KMdiChildView*)) );
   QObject::connect( b, SIGNAL(leftMouseButtonClicked(KMdiChildView*)), m_pFrm, SLOT(activateView(KMdiChildView*)) );
   QObject::connect( b, SIGNAL(rightMouseButtonClicked(KMdiChildView*)), m_pFrm, SLOT(taskbarButtonRightClicked(KMdiChildView*)) );
   QObject::connect( b, SIGNAL(buttonTextChanged(int)), this, SLOT(layoutTaskBar(int)) );
   m_pButtonList->append(b);
   b->setToggleButton( true);
   b->setText(win_ptr->tabCaption());

   layoutTaskBar();

   m_pStretchSpace = new QLabel(this, "empty");
   m_pStretchSpace->setText("");
   setStretchableWidget( m_pStretchSpace);
   m_pStretchSpace->show();

   if (m_bSwitchedOn) {
      b->show();
      show();
   }
   return b;
}

void KMdiTaskBar::removeWinButton(KMdiChildView *win_ptr, bool haveToLayoutTaskBar)
{
   KMdiTaskBarButton *b=getButton(win_ptr);
   if (b){
      m_pButtonList->removeRef(b);
      if( haveToLayoutTaskBar) layoutTaskBar();
   }
   if (m_pButtonList->count() == 0) {
      if (m_pStretchSpace != 0L) {
         delete m_pStretchSpace;
         m_pStretchSpace = 0L;
         hide();
      }
   }
}

void KMdiTaskBar::switchOn(bool bOn)
{
   m_bSwitchedOn = bOn;
   if (!bOn) {
      hide();
   }
   else {
      if (m_pButtonList->count() > 0) {
         show();
      }
      else {
         hide();
      }
   }
}

KMdiTaskBarButton * KMdiTaskBar::getButton(KMdiChildView *win_ptr)
{
   for(KMdiTaskBarButton *b=m_pButtonList->first();b;b=m_pButtonList->next()){
      if(b->m_pWindow == win_ptr)return b;
   }
   return 0;
}

KMdiTaskBarButton * KMdiTaskBar::getNextWindowButton(bool bRight,KMdiChildView *win_ptr)
{
   if(bRight){
      for(KMdiTaskBarButton *b=m_pButtonList->first();b;b=m_pButtonList->next()){
         if(b->m_pWindow == win_ptr){
            b = m_pButtonList->next();
            if(!b)b = m_pButtonList->first();
            if(win_ptr != b->m_pWindow)return b;
            else return 0;
         }
      }
   } else {
      for(KMdiTaskBarButton *b=m_pButtonList->first();b;b=m_pButtonList->next()){
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

void KMdiTaskBar::setActiveButton(KMdiChildView *win_ptr)
{
   KMdiTaskBarButton* newPressedButton = 0L;
   KMdiTaskBarButton* oldPressedButton = 0L;
   for(KMdiTaskBarButton *b=m_pButtonList->first();b;b=m_pButtonList->next()){
      if( b->m_pWindow == win_ptr)
         newPressedButton = b;
      if( b->m_pWindow == m_pCurrentFocusedWindow)
         oldPressedButton = b;
   }

   if( newPressedButton != 0L && newPressedButton != oldPressedButton) {
         if( oldPressedButton != 0L)
            oldPressedButton->toggle();// switch off
         newPressedButton->toggle();   // switch on
         m_pCurrentFocusedWindow = win_ptr;
   }
}

void KMdiTaskBar::layoutTaskBar( int taskBarWidth)
{
   if (m_layoutIsPending) return;
   m_layoutIsPending = true;

   if( !taskBarWidth)
      // no width is given
      taskBarWidth = width();

   // calculate current width of all taskbar buttons
   int allButtonsWidth = 0;
   KMdiTaskBarButton *b = 0;
   for(b=m_pButtonList->first();b;b=m_pButtonList->next()){
      allButtonsWidth += b->width();
   }

   // calculate actual width of all taskbar buttons
   int allButtonsWidthHint = 0;
   for(b=m_pButtonList->first();b;b=m_pButtonList->next()){
      QFontMetrics fm = b->fontMetrics();
      QString s = b->actualText();
      QSize sz = fm.size(ShowPrefix, s);
      int w = sz.width()+6;
      int h = sz.height()+sz.height()/8+10;
      w += h;
      allButtonsWidthHint += w;
   }

   // if there's enough space, use actual width
   int buttonCount = m_pButtonList->count();
   int tbHandlePixel;
   tbHandlePixel = style().pixelMetric(QStyle::PM_DockWindowHandleExtent, this);
   int buttonAreaWidth = taskBarWidth - tbHandlePixel - style().pixelMetric(QStyle::PM_DefaultFrameWidth, this) - 5;
   if( ((allButtonsWidthHint) <= buttonAreaWidth) || (width() < parentWidget()->width())) {
      for(b=m_pButtonList->first();b;b=m_pButtonList->next()){
         b->setText( b->actualText());
         if (b->width() != b->sizeHint().width()) {
            b->setFixedWidth( b->sizeHint().width());
            b->show();
         }
      }
   }
   else {
      // too many buttons for actual width
      int newButtonWidth;
      if( buttonCount != 0)
         newButtonWidth = buttonAreaWidth / buttonCount;
      else
         newButtonWidth = 0;
      if( orientation() == Qt::Vertical)
         newButtonWidth = 80;
      if(newButtonWidth > 0)
         for(b=m_pButtonList->first();b;b=m_pButtonList->next()){
            b->fitText( b->actualText(), newButtonWidth);
            if (b->width() != newButtonWidth) {
               b->setFixedWidth( newButtonWidth);
               b->show();
            }
         }
   }
   m_layoutIsPending = false;
}

void KMdiTaskBar::resizeEvent( QResizeEvent* rse)
{
   if (!m_layoutIsPending) {
      if (m_pButtonList->count() != 0) {
         layoutTaskBar( rse->size().width());
      }
   }
   KToolBar::resizeEvent( rse);
}
