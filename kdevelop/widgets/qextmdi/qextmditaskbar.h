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

#ifndef _QEXTMDITASKBAR_H_
#define _QEXTMDITASKBAR_H_

#include <qtoolbar.h>
#include <qlist.h>
#include <qpixmap.h>
#include <qpushbutton.h>
#include <qtoolbutton.h>

#include "qextmdidefines.h"

class QextMdiMainFrm;
class QextMdiChildView;
class QextMdiTaskBar;

class DLL_IMP_EXP_QEXTMDICLASS QextMdiTaskBarButton : public QToolButton
{
	Q_OBJECT

public:
	QextMdiTaskBarButton( QextMdiTaskBar *pTaskBar, QextMdiChildView *win_ptr);
	~QextMdiTaskBarButton();
	
	QextMdiChildView *m_pWindow;
signals:
   void clicked(QextMdiChildView*);
   void leftMouseButtonClicked(QextMdiChildView*);
   void rightMouseButtonClicked(QextMdiChildView*);
public slots:
   void setNewText(const QString&);
protected slots:
   void mousePressEvent( QMouseEvent*);
};

class QextMdiTaskBar : public QToolBar
{
	Q_OBJECT
public:		// Construction & Destruction
	QextMdiTaskBar(QextMdiMainFrm *parent,QMainWindow::ToolBarDock dock);
	~QextMdiTaskBar();
	QextMdiTaskBarButton * addWinButton(QextMdiChildView *win_ptr);
	void removeWinButton(QextMdiChildView *win_ptr);
//F.B.	void windowAttached(QextMdiChildView *win_ptr,bool bAttached);
	QextMdiTaskBarButton * getNextWindowButton(bool bRight,QextMdiChildView *win_ptr);
	QextMdiTaskBarButton * getButton(QextMdiChildView *win_ptr);
//F.B.	virtual void fontChange(const QFont &oldFont);
public slots:
	void setActiveButton(QextMdiChildView *win_ptr);
private:
	QList<QextMdiTaskBarButton>*  m_pButtonList;
	QextMdiMainFrm*               m_pFrm;
   QextMdiChildView*             m_pCurrentFocusedWindow;
};

#endif //_QEXTMDITASKBAR_H_

