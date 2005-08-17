//----------------------------------------------------------------------------
//    filename             : kmditaskbar.h
//----------------------------------------------------------------------------
//    Project              : KDE MDI extension
//
//    begin                : 07/1999       by Szymon Stefanek as part of kvirc
//                                         (an IRC application)
//    changes              : 09/1999       by Falk Brettschneider to create an
//                           - 06/2000     stand-alone Qt extension set of
//                                         classes and a Qt-based library
//                           2000-2003     maintained by the KDevelop project
//
//    copyright            : (C) 1999-2003 by Falk Brettschneider
//                                         and
//                                         Szymon Stefanek (stefanek@tin.it)
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

#ifndef _KMDITASKBAR_H_
#define _KMDITASKBAR_H_

#include <ktoolbar.h>
#include <q3ptrlist.h>
#include <qpixmap.h>
#include <qlabel.h>
#include <qpushbutton.h>
//Added by qt3to4:
#include <QMouseEvent>
#include <QResizeEvent>

#include "kmdidefines.h"

class KMdiMainFrm;
class KMdiChildView;
class KMdiTaskBar;

class KMdiTaskBarButtonPrivate;

/**
  * @short Internal class.
  *
  * It's a special kind of QPushButton catching mouse clicks.
  * And you have the ability to abbreviate the text that it fits in the button.
  */
class KMdiTaskBarButton : public QPushButton
{
   Q_OBJECT
// methods
public:
   /**
   * Constructor (sets to toggle button, adds a tooltip (caption) and sets to NoFocus
   */
   KMdiTaskBarButton( KMdiTaskBar *pTaskBar, KMdiChildView *win_ptr);
   /**
   * Destructor
   */
   ~KMdiTaskBarButton();
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
   void clicked(KMdiChildView*);
   /**
   * Internally connected with KMdiMainFrm::activateView
   */
   void leftMouseButtonClicked(KMdiChildView*);
   /**
   * Internally connected with KMdiMainFrm::taskbarButtonRightClicked
   */
   void rightMouseButtonClicked(KMdiChildView*);
   /**
   * Emitted when the button text has changed. Internally connected with KMdiTaskBar::layoutTaskBar
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
   KMdiChildView *m_pWindow;
protected:
   /**
   * Internally we must remember the real text because the button text can be abbreviated.
   */
   QString m_actualText;

private:
   KMdiTaskBarButtonPrivate *d;
};


class KMdiTaskBarPrivate;
/**
 * @short Internal class.
 *
 * It's a special kind of QToolBar that acts as taskbar for child views.
 * KMdiTaskBarButtons can be added or removed dynamically.<br>
 * The button sizes are adjusted dynamically, as well.
 */
class KMDI_EXPORT KMdiTaskBar : public KToolBar
{
   Q_OBJECT
public:
   /**
   * Constructor (NoFocus, minimum width = 1, an internal QPtrList of taskbar buttons (autodelete))
   */
   KMdiTaskBar(KMdiMainFrm *parent,Qt::ToolBarDock dock);
   /**
   * Destructor (deletes the taskbar button list)
   */
   ~KMdiTaskBar();
   /**
   *Add a new KMdiTaskBarButton . The width doesn't change.
   * If there's not enough space, all taskbar buttons will be resized to a new smaller size.
   * Probably button texts must be abbreviated now.
   */
   KMdiTaskBarButton * addWinButton(KMdiChildView *win_ptr);
   /**
   * Removes a KMdiTaskBarButton and deletes it. If the rest of the buttons are smaller
   * than they usually are, all those buttons will be resized in a way that the new free size is used as well.
   */
   void removeWinButton(KMdiChildView *win_ptr, bool haveToLayoutTaskBar = true);
   /**
   * Returns the neighbor taskbar button of the taskbar button of the MDI view given by parameter
   * bRight specifies the side, of course left is used if bRight is false.
   */
   KMdiTaskBarButton * getNextWindowButton(bool bRight,KMdiChildView *win_ptr);
   /**
   * Get the button belonging to the MDI view given as parameter.
   */
   KMdiTaskBarButton * getButton(KMdiChildView *win_ptr);
   /**
   * Switch it on or off.
   */
   void switchOn(bool bOn);
   /**
   * @return whether switched on or off.
   */
   bool isSwitchedOn() const { return m_bSwitchedOn; };
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
   * Actually it's a radiobutton group behavior.
   */
   void setActiveButton(KMdiChildView *win_ptr);
protected:
   /**
   * A list of taskbar buttons.
   * Note: Each button stands for one MDI view (toolviews doesn't have got a taskbar button).
   */
   Q3PtrList<KMdiTaskBarButton>*  m_pButtonList;
   /**
   * The belonging MDI mainframe (parent widget of this)
   */
   KMdiMainFrm*               m_pFrm;
   /**
   * The MDI view belonging to the currently pressed taskbar button
   */
   KMdiChildView*             m_pCurrentFocusedWindow;
   /**
   * A stretchable widget used as 'space' at the end of a half filled taskbar
   */
   QLabel*                       m_pStretchSpace;
   bool                          m_layoutIsPending;
   bool                          m_bSwitchedOn;

private:
   KMdiTaskBarPrivate *d;
};

#endif //_KMDITASKBAR_H_
