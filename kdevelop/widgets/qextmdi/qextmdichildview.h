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

#ifndef _QEXTMDICHILDVIEW_H_
#define _QEXTMDICHILDVIEW_H_

#include <qwidget.h>
#include <qpixmap.h>
#include <qrect.h>
#include <qpushbutton.h>   //F.B.
#include <qtoolbutton.h>   //F.B.

#include "qextmdichildfrm.h"

/**
 * @short Base class for all interface windows
 * Base class for all interface windows.<br>
 * Defines some virtual functions for later common use.<br>
 * The derived windows 'lives' attached to a QextMdiChildFrm widget<br>
 * managed by KviMdiManager , or detached (managed by the window manager.)<br>
 * So remember that the parent() pointer may change , and may be 0 too.<br>
 */

class DLL_IMP_EXP_QEXTMDICLASS QextMdiChildView : public QWidget
{
	Q_OBJECT

public:		// Consruction & Destruction
	QextMdiChildView(const QString& name, QWidget* parentWidget = 0);
	~QextMdiChildView();
protected:		// Fields
	QString            m_szCaption;

public:     // Methods
	/**
	 * Returns true if this window is attached to the Mdi manager<br>
	 * (inline)
	 */
	bool isAttached();
	/**
	 * Returns the caption text<br>
	 * (inline)
	 */
	const QString& caption();
	/**
	 * Sets the window caption string...<br>
	 * Calls updateButton on the taskbar button if it has been set.<br>
	 */
	void setWindowCaption(const QString& szCaption);
	/**
	 * Highlights the related taskbar button.<br>
	 * Should be called from all do*Output overridden calls.<br>
	 * (inline)
	 */
//F.B.	void highlight();
	/**
	 * Returns the QextMdiChildFrm parent widget (or 0 if the window is not attached)
	 */
	QextMdiChildFrm *mdiParent();
	/**
	 * Tells if the window is minimized when attached to the Mdi manager,<br>
	 * or if it is VISIBLE when 'floating'.
	 */
	bool isMinimized();//Useful only when attached (?)
	/**
	 * Tells if the window is minimized when attached to the Mdi manager,<br>
	 * otherwise returns false.
	 */
	bool isMaximized();//Useful only when attached (?)
	/**
	 * Returns the geometry of this window or of the parent if there is any...
	 */
	QRect externalGeometry();
	//Methods to override ABSOLUTELY
	/**
	 * You SHOULD override this function in the derived class<br>
	 * and return one of the g_pWindowIcon pointers managed by KviApp.
	 */
	virtual QPixmap * myIconPtr();
	/**
	 * Must be overridden in child classes.
	 */
//F.B.	virtual void applyOptions();
	/**
	 * Sets the related taskbar button progress
	 */
//F.B.	void setProgress(int progress);
	/**
	 * Minimizes this window when it is attached to the Mdi manager.<br>
	 * Otherwise has no effect
	 */
	void minimize(bool bAnimate);   //Useful only when attached
	/**
	 * Maximizes this window when it is attached to the Mdi manager.<br>
	 * Otherwise has no effect
	 */
	void maximize(bool bAnimate);   //Useful only when attached
	
/*F.B.	virtual void setProperties(QextMdiChildViewProperty * p); //do nothing here....
	  virtual void saveProperties();F.B.*/
	
	public slots:
	  /**
	   * Attaches this window to the Mdi manager.<br>
	   * It calls the KviFrame attachWindow function , so if you have a pointer<br>
	   * to this KviFrame you'll be faster calling that function.<br>
	   * Useful as slot.
	   */
	  void attach();
	/**
	 * Detaches this window from the Mdi manager.<br>
	 * It calls the KviFrame detachWindow function , so if you have a pointer<br>
	 * to this KviFrame you'll be faster calling that function.<br>
	 * Useful as slot.
	 */
	void detach();
	
	void minimize(); //Overload and slot
	void maximize(); //Overload and slot
	/**
	 * Restores this window when it is attached to the Mdi manager.
	 */
	void restore();    //Useful only when attached
	void youAreAttached(QextMdiChildFrm *lpC);
	void youAreDetached();
 protected:	// Protected methods
	/**
	 * Ignores the event and calls KviFrame::childWindowCloseRequest
	 * @see KviFrame::childWindowCloseRequest
	 */
	virtual void closeEvent(QCloseEvent *e);
//F.B.	virtual bool eventFilter(QObject *o,QEvent *e);
//F.B.  virtual void resizeEvent(QResizeEvent *e);   //F.B.
      	virtual void focusInEvent(QFocusEvent *); //F.B.
	
 signals:
	void attachWindow( QextMdiChildView*,bool,bool,QRect*);
	void detachWindow( QextMdiChildView*);
	void focusInEventOccurs( QextMdiChildView*);
	void childWindowCloseRequest( QextMdiChildView*);
	void windowCaptionChanged( const QString&);
};

inline bool QextMdiChildView::isAttached(){ return (parent() != 0); }
inline const QString& QextMdiChildView::caption(){ return m_szCaption; }
inline QextMdiChildFrm *QextMdiChildView::mdiParent(){ return (QextMdiChildFrm *)parent(); }

#endif //_QEXTMDICHILDVIEW_H_
