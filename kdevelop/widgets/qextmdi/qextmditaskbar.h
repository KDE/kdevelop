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

#include <qtoolbar.h>
#include <qlist.h>
#include <qpixmap.h>
#include <qlabel.h>
#include <qpushbutton.h>

#include "qextmdidefines.h"

class QextMdiMainFrm;
class QextMdiChildView;
class QextMdiTaskBar;

/**
  * @short Internal class.
  * It's a special kind of QPushButton catching mouse clicks.
  * And you have the ability to abbreviate the text that it fits in the button.
  */
class DLL_IMP_EXP_QEXTMDICLASS QextMdiTaskBarButton : public QPushButton
{
   Q_OBJECT
// methods
public:
   QextMdiTaskBarButton( QextMdiTaskBar *pTaskBar, QextMdiChildView *win_ptr);
   ~QextMdiTaskBarButton();
   QString actualText() const;
   void fitText(const QString&, int newWidth);
   void setText(const QString&);
signals:
   void clicked(QextMdiChildView*);
   void leftMouseButtonClicked(QextMdiChildView*);
   void rightMouseButtonClicked(QextMdiChildView*);
   void buttonTextChanged(int);
public slots:
   void setNewText(const QString&);
protected slots:
   void mousePressEvent( QMouseEvent*);

// attributes
public:
   QextMdiChildView *m_pWindow;
protected:
   QString m_actualText;
};

/**
 * @short Internal class.
 * It's a special kind of QToolBar that acts as taskbar for child views.
 * QextMdiTaskBarButtons can be added or removed dynamically.<br>
 * The button sizes are adjusted dynamically, as well.
 */
class DLL_IMP_EXP_QEXTMDICLASS QextMdiTaskBar : public QToolBar
{
   Q_OBJECT
public:     // Construction & Destruction
   QextMdiTaskBar(QextMdiMainFrm *parent,QMainWindow::ToolBarDock dock);
   ~QextMdiTaskBar();
   QextMdiTaskBarButton * addWinButton(QextMdiChildView *win_ptr);
   void removeWinButton(QextMdiChildView *win_ptr, bool haveToLayoutTaskBar = TRUE);
   QextMdiTaskBarButton * getNextWindowButton(bool bRight,QextMdiChildView *win_ptr);
   QextMdiTaskBarButton * getButton(QextMdiChildView *win_ptr);
protected:
   void resizeEvent( QResizeEvent*);
protected slots:
   void layoutTaskBar( int taskBarWidth = 0);
public slots:
   void setActiveButton(QextMdiChildView *win_ptr);
private:
   QList<QextMdiTaskBarButton>*  m_pButtonList;
   QextMdiMainFrm*               m_pFrm;
   QextMdiChildView*             m_pCurrentFocusedWindow;
   QLabel*                       m_pStretchSpace;
};

#endif //_QEXTMDITASKBAR_H_
