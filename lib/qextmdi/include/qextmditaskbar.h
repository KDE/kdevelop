//----------------------------------------------------------------------------
//    filename             : qextmditaskbar.h
//----------------------------------------------------------------------------
//    Project              : Qt MDI extension
//
//    begin                : 07/1999       by Szymon Stefanek as part of kvirc
//                                         (an IRC application)
//    changes              : 09/1999       by Falk Brettschneider to create an
//                           - 06/2000     stand-alone Qt extension set of
//                                         classes and a Qt-based library
//
//    copyright            : (C) 1999-2000 by Falk Brettschneider
//                                         and
//                                         Szymon Stefanek (stefanek@tin.it)
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

#ifndef _QEXTMDITASKBAR_H_
#define _QEXTMDITASKBAR_H_

#ifdef NO_KDE2
#include "dummyktoolbar.h"
#else
#include <ktoolbar.h>
#endif
#if QT_VERSION < 300
# include <qlist.h>
#else
# include <qptrlist.h>
#endif
#include <qpixmap.h>
#include <qlabel.h>
#include <qpushbutton.h>

#include "qextmdidefines.h"

class QextMdiMainFrm;
class QextMdiChildView;
class QextMdiTaskBar;

/**
  * @short Internal class.
  *
  * It's a special kind of QPushButton catching mouse clicks.
  * And you have the ability to abbreviate the text that it fits in the button.
  */
class DLL_IMP_EXP_QEXTMDICLASS QextMdiTaskBarButton : public QPushButton
{
   Q_OBJECT
// methods
public:
   /** 
   * Constructor (sets to toggle button, adds a tooltip (caption) and sets to NoFocus 
   */
   QextMdiTaskBarButton( QextMdiTaskBar *pTaskBar, QextMdiChildView *win_ptr);
   /** 
   * Destructor 
   */
   ~QextMdiTaskBarButton();
   /** 
   * text() returns the possibly abbreviated text including the dots in it. But actualText() returns the full text. 
   */
   QString actualText() const;
   /** 
   * Given the parameter newWidth this function possibly abbreviates the parameter string and sets a new button text. 
   */
   void fitText(const QString&, int newWidth);
   /** 
   * Sets the text and avoids any abbreviation. Memorizes that text in m_actualText, too. 
   */
   void setText(const QString&);
signals:
   /** 
   * Emitted when the button has been clicked. Internally connected to setFocus of the according MDI view. 
   */
   void clicked(QextMdiChildView*);
   /** 
   * Internally connected with @ref QextMdiMainFrm::activateView 
   */
   void leftMouseButtonClicked(QextMdiChildView*);
   /** 
   * Internally connected with @ref QextMdiMainFrm::taskbarButtonRightClicked 
   */
   void rightMouseButtonClicked(QextMdiChildView*);
   /** 
   * Emitted when the button text has changed. Internally connected with @ref QextMdiTaskBar::layoutTaskBar 
   */
   void buttonTextChanged(int);
public slots:
   /** 
   * A slot version of setText 
   */
   void setNewText(const QString&);
protected slots:
   /** 
   * Reimplemented from its base class to catch right and left mouse button clicks 
   */
   void mousePressEvent( QMouseEvent*);

// attributes
public:
   /** 
   * The according MDI view 
   */
   QextMdiChildView *m_pWindow;
protected:
   /** 
   * Internally we must remember the real text because the button text can be abbreviated. 
   */
   QString m_actualText;
};

/**
 * @short Internal class.
 *
 * It's a special kind of QToolBar that acts as taskbar for child views.
 * QextMdiTaskBarButtons can be added or removed dynamically.<br>
 * The button sizes are adjusted dynamically, as well.
 */
class DLL_IMP_EXP_QEXTMDICLASS QextMdiTaskBar : public KToolBar
{
   Q_OBJECT
public:
   /** 
   * Constructor (NoFocus, minimum width = 1, an internal QList of taskbar buttons (autodelete)) 
   */
   QextMdiTaskBar(QextMdiMainFrm *parent,QMainWindow::ToolBarDock dock);
   /** 
   * Destructor (deletes the taskbar button list) 
   */
   ~QextMdiTaskBar();
   /** 
   *Add a new @ref QextMdiTaskBarButton . The width doesn't change.
   * If there's not enough space, all taskbar buttons will be resized to a new smaller size.
   * Probably button texts must be abbreviated now. 
   */
   QextMdiTaskBarButton * addWinButton(QextMdiChildView *win_ptr);
   /** 
   * Removes a @ref QextMdiTaskBarButton and deletes it. If the rest of the buttons are smaller
   * than they usually are, all those buttons will be resized in a way that the new free size is used as well. 
   */
   void removeWinButton(QextMdiChildView *win_ptr, bool haveToLayoutTaskBar = TRUE);
   /** 
   * Returns the neighbor taskbar button of the taskbar button of the MDI view given by parameter
   * bRight specifies the side, of course left is used if bRight is false. 
   */
   QextMdiTaskBarButton * getNextWindowButton(bool bRight,QextMdiChildView *win_ptr);
   /** 
   * Get the button belonging to the MDI view given as parameter. 
   */
   QextMdiTaskBarButton * getButton(QextMdiChildView *win_ptr);
   /**
   * Switch it on or off.
   */
   void switchOn(bool bOn);
   /**
   * @return whether switched on or off.
   */
   bool isSwitchedOn() { return m_bSwitchedOn; };
protected:
   /** 
   * Reimplemented from its base class to call layoutTaskBar, additionally. 
   */
   void resizeEvent( QResizeEvent*);
protected slots:
   /** 
   * Checks if all buttons fits into this. If not, it recalculates all button widths
   * in a way that all buttons fits into the taskbar and have got equal width.
   * The text of the buttons will be abbreviated when nessecary, all buttons get a
   * fixed width and show() is called for each one.
   * If one drags the taskbar to a vertical orientation, the button width is set to 80 pixel. 
   */
   void layoutTaskBar( int taskBarWidth = 0);
public slots:
   /** 
   * Pushes the desired taskbar button down (switch on), the old one is released (switched off).
   * Actually it's a radiobutton group behaviour. 
   */
   void setActiveButton(QextMdiChildView *win_ptr);
protected:
   /** 
   * A list of taskbar buttons.
   * Note: Each button stands for one MDI view (toolviews doesn't have got a taskbar button). 
   */
#if QT_VERSION < 300
   QList<QextMdiTaskBarButton>*  m_pButtonList;
#else
   QPtrList<QextMdiTaskBarButton>*  m_pButtonList;
#endif
   /**
   * The belonging MDI mainframe (parent widget of this)
   */
   QextMdiMainFrm*               m_pFrm;
   /** 
   * The MDI view belonging to the currently pressed taskbar button 
   */
   QextMdiChildView*             m_pCurrentFocusedWindow;
   /** 
   * A stretchable widget used as 'space' at the end of a half filled taskbar 
   */
   QLabel*                       m_pStretchSpace;
   bool                          m_layoutIsPending;
   bool                          m_bSwitchedOn;
};

#endif //_QEXTMDITASKBAR_H_
