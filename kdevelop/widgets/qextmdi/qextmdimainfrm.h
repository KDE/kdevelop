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

#ifndef _QEXTMDIMAINFRM_H_
#define _QEXTMDIMAINFRM_H_

#include <qmainwindow.h>
#include <qlist.h>
#include <qrect.h>
#include <qpopupmenu.h>
#include <qapplication.h>

#include "qextmditaskbar.h"
#include "qextmdichildarea.h"
#include "qextmdichildview.h"

class QextMdiViewCloseEvent : public QCustomEvent
{
public:
#if QT_VERSION > 144
	QextMdiViewCloseEvent( QextMdiChildView* pWnd) : QCustomEvent(QEvent::User,pWnd) {};
#else
	QextMdiViewCloseEvent( QextMdiChildView* pWnd) : QCustomEvent(Event_User,pWnd) {};
#endif
};

class DLL_IMP_EXP_QEXTMDICLASS QextMdiMainFrm : public QMainWindow
{
	friend class QextMdiChildView;
	friend class QextMdiTaskBar;
	
	Q_OBJECT

public:		// Consruction & Destruction
	QextMdiMainFrm( QWidget* parentWidget, const char* name = "", WFlags flags = WType_TopLevel);
	~QextMdiMainFrm();

public:		// Fields //why don't try to make it private?
	//Bars
//F.B.	KviMenuBar    *m_pMenuBar;
//F.B.	KviToolBar    *m_pToolBar;
//F.B.	KviStatusBar  *m_pStatusBar;
	QextMdiTaskBar    *m_pTaskBar;
	//Mdi manager
	QextMdiChildArea *m_pMdi;
	QList<QextMdiChildView> *m_pWinList;
	//Console window
	QextMdiChildView* m_pCurrentWindow;

   QPopupMenu* m_pWindowPopup;
   QPopupMenu* m_pTaskBarPopup;
   QextMdiChildView*  m_pCurrentActiveWindow;
public:
	// Window management
	QextMdiChildView * activeWindow();
	QPopupMenu * taskBarPopup(QextMdiChildView *pWnd,bool bIncludeWindowPopup = false);
	QPopupMenu * windowPopup(QextMdiChildView *pWnd,bool bIncludeTaskbarPopup = true);
	void applyOptions();
	QextMdiChildView * findWindow(const QString& caption);
	bool windowExists(QextMdiChildView *pWnd);
	void switchWindows(bool bRight);
  virtual	bool event(QEvent* e);
public slots:
	void addWindow(QextMdiChildView *pWnd,bool bShow, bool bAttach = true);
   void removeWindowFromMdi(QextMdiChildView *pWnd);
	void closeWindow(QextMdiChildView *pWnd);
	// toolbar slots
	void slot_toggleStatusBar();
	void slot_toggleTaskBar();
	void slot_toggleToolBar();
protected:
	//Connection management
	void applyToolbarOptions();
	bool checkHighlight(const QString& msg);
	//Socket events
	virtual void closeEvent(QCloseEvent *e);
	virtual void focusInEvent(QFocusEvent *);
	void childWindowGainFocus(QextMdiChildView *pWnd);
	//F.B.void windowPopupRequested(QextMdiChildView *pWnd);
	// Creation
	void createMenuBar();
	void createToolBar();
	void createTaskBar();
	void createStatusBar();
	void createMdiManager();
   void raiseTopLevelWidget(QWidget * ptr);  //added by F.B.
   QPopupMenu* windowMenu() { return m_pMdi->m_pWindowMenu; };
public slots:	
	void attachWindow(QextMdiChildView *pWnd,bool bShow,bool overrideGeometry,QRect *r);
	void detachWindow(QextMdiChildView *pWnd);
	void childWindowCloseRequest(QextMdiChildView *pWnd);
protected slots:
	void taskbarButtonLeftClicked(QextMdiChildView *pWnd);
	void taskbarButtonRightClicked(QextMdiChildView *pWnd);
	void pushNewTaskBarButton(QextMdiChildView* pWnd);
};

#endif //_QEXTMDIMAINFRM_H_
