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
//    copyright            : (C) 1999-2000 by Falk Brettschneider
//                                         and
//                                         Szymon Stefanek (stefanek@tin.it)
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

#ifndef _QEXTMDICHILDFRM_H_
#define _QEXTMDICHILDFRM_H_

#include <qframe.h>
#include <qlist.h>
#include <qpixmap.h>
#include <qpopupmenu.h>
#include <qpushbutton.h>
#include <qtoolbutton.h>
#include <qdict.h>

#include "qextmdichildfrmcaption.h" //cross ref

class QextMdiChildArea;
class QextMdiChildView;

/**
  * @short Internal class.
  * It's an MDI child frame widget. It contains a view widget and a frame caption. Usually you derive from its view.
  */

class DLL_IMP_EXP_QEXTMDICLASS QextMdiChildFrm : public QFrame
{
	friend class QextMdiChildArea;
	friend class QextMdiChildFrmCaption;
	Q_OBJECT
public:
	enum MdiWindowState { Normal,Maximized,Minimized };
	/**
	* Creates a new QextMdiChildFrm class.<br>
	*/
	QextMdiChildFrm(QextMdiChildArea *parent);
	/**
	* Delicato : destroys this QextMdiChildFrm
	* If a child is still here managed (no recreation was made) it is destroyed too.
	*/
	~QextMdiChildFrm();	
public:
	QextMdiChildView       *m_pClient;
protected:
	QextMdiChildArea       *m_pManager;
	QextMdiChildFrmCaption *m_pCaption;
#ifdef WIN32
	/**
	* This is a POINTER to an icon 16x16. If this is 0 no icon is painted.
	*/
	QPushButton   *m_pIcon;
	QPushButton   *m_pMinimize;
	QPushButton   *m_pMaximize;
	QPushButton   *m_pClose;
	QPushButton   *m_pUndock;
#else	// in case of UNIX: KDE look
	QToolButton   *m_pIcon;
	QToolButton   *m_pMinimize;
	QToolButton   *m_pMaximize;
	QToolButton   *m_pClose;
	QToolButton   *m_pUndock;
#endif
	MdiWindowState m_state;
	QRect          m_restoredRect;
	int            m_iResizeCorner;
	int            m_iLastCursorCorner;
	bool           m_resizeMode;
   QPixmap*       m_pIconButtonPixmap;
   QPixmap*       m_pMinButtonPixmap;
   QPixmap*       m_pMaxButtonPixmap;
   QPixmap*       m_pRestoreButtonPixmap;
   QPixmap*       m_pCloseButtonPixmap;
   /** every child frame window has an temporary ID in the Window menu of the main frame. */
   int 				m_windowMenuID;
   QPixmap*       m_pUndockButtonPixmap;
public:
	/**
	* Reparents the widget w to this QextMdiChildFrm (if this is not already done)
	* Installs an event filter to catch focus events.
	* Resizes this mdi child in a way that the child fits perfectly in.
	*/
	void setClient(QextMdiChildView *w);
	/**
	* Reparents the client widget to 0 (desktop)
	* Removes the event filter.
	*/
	void unsetClient();
	/**
	* Sets the window icon ponter : assumes 16x16 pixmaps that SHOULD BE NOT DELETED
	* until this object is destroyed.
	*/
	void setIcon(QPixmap *ptr);
	/**
	* Enables or disables the close button
	*/
	void enableClose(bool bEnable);
	/**
	* Sets the caption of this window
	*/
	void setCaption(const QString& text);
	/**
	* Returns the caption of this mdi child.
	* Cool to have it inline...
	*/
	const QString& caption(){ return m_pCaption->m_szCaption; };
	/**
	* Minimizes , Maximizes or restores the window.
	*/
	void setState(MdiWindowState state,bool bAnimate=true);
	/**
	* Returns the current state of the window
	* Cool to have it inline...
	*/
	inline MdiWindowState state(){ return m_state; };
//	virtual void hide(){ setState(Minimized,false); };
//	virtual void show(){ setState(Normal,false); };
	/**
	* Forces updating the rects of the caption and so...
	* It may be useful when setting the mdiCaptionFont of the MdiManager
	*/
	void updateRects(){ resizeEvent(0); };
  /** sets an ID that is submitted to QextMdiChildArea::menuActivated( int) when the menu item for this child frame was clicked */
  void setWindowMenuID( int id);
private:
	void linkChildren( QDict<FocusPolicy>* pFocPolDict);
	QDict<QWidget::FocusPolicy>* unlinkChildren();
	int getResizeCorner(int ax,int ay);
protected slots:
	void maximizePressed();
	void minimizePressed();
	void closePressed();
	void undockPressed();
	void raiseAndActivate();
protected:
	virtual void resizeEvent(QResizeEvent *);
	virtual void mouseMoveEvent(QMouseEvent *e);
	virtual void mousePressEvent(QMouseEvent *e);
	virtual void mouseReleaseEvent(QMouseEvent *);
   virtual void leaveEvent(QEvent *);
   virtual bool eventFilter(QObject*, QEvent*);//focusInEvent(QFocusEvent *);
  //   virtual bool focusNextPrevChild( bool next ) { return true; };
	void resizeWindow(int resizeCorner, int x, int y);
	void moveWindow(QPoint diff, QPoint relativeMousePos);
	void setResizeCursor(int resizeCorner);
public slots: // Public slots
  /** called if someone click on the "Window" menu item for this child frame window */
  void slot_clickedInWindowMenu();
signals: // Signals
  /** is automatically emitted when slot_clickedInWindowMenu is called */
  void clickedInWindowMenu(int);
  /** forces a mouse release event call */
  void releaseMouse();
};


#endif //_QEXTMDICHILDFRM_H_
