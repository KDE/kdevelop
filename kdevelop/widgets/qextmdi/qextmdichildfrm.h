//----------------------------------------------------------------------------
//    filename             : qextmdichildfrm.h
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

#ifndef _QEXTMDICHILDFRM_H_
#define _QEXTMDICHILDFRM_H_

#include <qframe.h>
#include <qlist.h>
#include <qpixmap.h>
#include <qpopupmenu.h>
#include <qpushbutton.h>
#include <qtoolbutton.h>
#include <qlabel.h>
#include <qdict.h>

#include "qextmdichildfrmcaption.h" //cross ref

class QextMdiChildArea;
class QextMdiChildView;

/**
  * @short Internal class, only used on Win32.
  * This class provides a label widget that can process mouse click events.
  */
class DLL_IMP_EXP_QEXTMDICLASS QextMdiWin32IconButton : public QLabel
{
	Q_OBJECT
public:
	QextMdiWin32IconButton( QWidget* parent, const char* name = 0);
	virtual void mousePressEvent( QMouseEvent*);

signals:
	void pressed();
};

/**
  * @short Internal class.
  * It's an MDI child frame widget. It contains a view widget and a frame caption. Usually you derive from its view.
  */
class DLL_IMP_EXP_QEXTMDICLASS QextMdiChildFrm : public QFrame
{
	friend class QextMdiChildArea;
	friend class QextMdiChildFrmCaption;
	Q_OBJECT

// attributes	
public:
	enum MdiWindowState { Normal,Maximized,Minimized };
	QextMdiChildView        *m_pClient;

protected:
	QextMdiChildArea        *m_pManager;
	QextMdiChildFrmCaption  *m_pCaption;
#ifdef _OS_WIN32_
	/**
	* This is a POINTER to an icon 16x16. If this is 0 no icon is painted.
	*/
	QextMdiWin32IconButton  *m_pIcon;
	QPushButton    *m_pMinimize;
	QPushButton    *m_pMaximize;
	QPushButton    *m_pClose;
	QPushButton    *m_pUndock;
#else	// in case of UNIX: KDE look
	QToolButton    *m_pIcon;
	QToolButton    *m_pMinimize;
	QToolButton    *m_pMaximize;
	QToolButton    *m_pClose;
	QToolButton    *m_pUndock;
#endif
	MdiWindowState m_state;
	QRect          m_restoredRect;
	int            m_iResizeCorner;
	int            m_iLastCursorCorner;
	bool           m_resizeMode;
   QPixmap        *m_pIconButtonPixmap;
   QPixmap        *m_pMinButtonPixmap;
   QPixmap        *m_pMaxButtonPixmap;
   QPixmap        *m_pRestoreButtonPixmap;
   QPixmap        *m_pCloseButtonPixmap;
   QPixmap        *m_pUndockButtonPixmap;
   /** every child frame window has an temporary ID in the Window menu of the main frame. */
   int 				m_windowMenuID;
   /** imitates a system menu for child frame windows */
   QPopupMenu     *m_pSystemMenu;
   QSize          m_oldClientMinSize;

// methods
public:
	/**
	* Creates a new QextMdiChildFrm class.<br>
	*/
	QextMdiChildFrm(QextMdiChildArea *parent);
	/**
	* Delicato : destroys this QextMdiChildFrm
	* If a child is still here managed (no recreation was made) it is destroyed too.
	*/
	~QextMdiChildFrm();	
	/**
	* Reparents the widget w to this QextMdiChildFrm (if this is not already done)
	* Installs an event filter to catch focus events.
	* Resizes this mdi child in a way that the child fits perfectly in.
	*/
	void setClient(QextMdiChildView *w);
	/**
	* Reparents the client widget to 0 (desktop), moves with an offset from the original position
	* Removes the event filter.
	*/
	void unsetClient( QPoint positionOffset = QPoint(0,0));
	/**
	* Sets the window icon ponter : assumes 16x16 pixmaps that SHOULD BE NOT DELETED
	* until this object is destroyed.
	*/
	void setIcon(QPixmap *ptr);
	/**
	 * Returns the child frame icon.
	 */
	QPixmap* icon();
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
	void setState(MdiWindowState state,bool bAnimate=TRUE);
	/**
	* Returns the current state of the window
	* Cool to have it inline...
	*/
	inline MdiWindowState state(){ return m_state; };
//	virtual void hide(){ setState(Minimized,FALSE); };
//	virtual void show(){ setState(Normal,FALSE); };
	/**
	* Forces updating the rects of the caption and so...
	* It may be useful when setting the mdiCaptionFont of the MdiManager
	*/
	void updateRects(){ resizeEvent(0); };
   /** sets an ID that is submitted to QextMdiChildArea::menuActivated( int) when the menu item for this child frame was clicked */
   void setWindowMenuID( int id);
   /** Returns the system menu. */
   QPopupMenu* systemMenu();

public slots: // Public slots
   /** called if someone click on the "Window" menu item for this child frame window */
   void slot_clickedInWindowMenu();
   void slot_resizeViaSystemMenu();

protected:
	virtual void resizeEvent(QResizeEvent *);
	virtual void mouseMoveEvent(QMouseEvent *e);
	virtual void mousePressEvent(QMouseEvent *e);
	virtual void mouseReleaseEvent(QMouseEvent *);
   virtual void leaveEvent(QEvent *);
   virtual bool eventFilter(QObject*, QEvent*);//focusInEvent(QFocusEvent *);
   //   virtual bool focusNextPrevChild( bool next ) { return TRUE; };
	void resizeWindow(int resizeCorner, int x, int y);
	void setResizeCursor(int resizeCorner);
   virtual void switchToMinimizeLayout();

protected slots:
	void maximizePressed();
	void restorePressed();
	void minimizePressed();
	void closePressed();
	void undockPressed();
	void raiseAndActivate();
   /** Shows a system menu for child frame windows. */
   void showSystemMenu();

private:
	void linkChildren( QDict<FocusPolicy>* pFocPolDict);
	QDict<QWidget::FocusPolicy>* unlinkChildren();
	int getResizeCorner(int ax,int ay);

signals: // Signals
   /** is automatically emitted when slot_clickedInWindowMenu is called */
   void clickedInWindowMenu(int);
};


#endif //_QEXTMDICHILDFRM_H_
