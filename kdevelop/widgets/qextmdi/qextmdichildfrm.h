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

#ifndef _QEXTMDICHILDFRM_H_
#define _QEXTMDICHILDFRM_H_

//#include <qscrollview.h> //tentative
#include <qframe.h>
#include <qlist.h>
#include <qpixmap.h>
#include <qpopupmenu.h>
#include <qpushbutton.h>

#include "qextmdichildfrmcaption.h" //cross ref

class QextMdiChildArea;
class QextMdiChildView;

/**
* A MDI child widget: Usually you don't need to derive from this class.
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
	QextMdiChildView     *m_pClient;
protected:
	QextMdiChildArea *m_pManager;
	QextMdiChildFrmCaption *m_pCaption;
	QPushButton *m_pMinimize;
	QPushButton *m_pMaximize;
	QPushButton *m_pClose;
	QPushButton *m_pUndock;
	MdiWindowState m_state;
	QRect          m_restoredRect;
	int            m_iResizeCorner;
	int            m_iLastCursorCorner;
	bool           m_resizeMode;  //F.B.
   QPixmap*       m_pMinButtonPixmap;
   QPixmap*       m_pMaxButtonPixmap;
   QPixmap*       m_pRestoreButtonPixmap;
   QPixmap*       m_pCloseButtonPixmap;
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
	void setIconPointer(QPixmap *ptr);
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
private:
	void linkChildren();
	void unlinkChildren();
	int getResizeCorner(int ax,int ay);
protected slots:
	void maximizePressed();
	void minimizePressed();
	void closePressed();
	void undockPressed();
protected:
	virtual void resizeEvent(QResizeEvent *);
	virtual void mouseMoveEvent(QMouseEvent *e);
	virtual void mousePressEvent(QMouseEvent *e);
	virtual void mouseReleaseEvent(QMouseEvent *);
   virtual void leaveEvent(QEvent *);
   virtual bool eventFilter(QObject*, QEvent*);//focusInEvent(QFocusEvent *);
	void resizeWindow(int resizeCorner/*F.B.*/,int x, int y /*F.B.*/);
	void moveWindow(QPoint diff);
	void setResizeCursor(int resizeCorner);
};


#endif //_QEXTMDICHILDFRM_H_
















