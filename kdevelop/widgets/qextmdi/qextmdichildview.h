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

#ifndef _QEXTMDICHILDVIEW_H_
#define _QEXTMDICHILDVIEW_H_

#include <qwidget.h>
#include <qpixmap.h>
#include <qrect.h>
#include <qapplication.h>

#include "qextmdichildfrm.h"

/**
  * @short Base class for all your special view windows.
  * Base class for all interface windows.<br>
  * Defines some virtual functions for later common use.<br>
  * The derived windows 'lives' attached to a QextMdiChildFrm widget<br>
  * managed by QextMdiChildArea, or detached (managed by the window manager.)<br>
  * So remember that the parent() pointer may change , and may be 0 too.<br>
  */

class DLL_IMP_EXP_QEXTMDICLASS QextMdiChildView : public QWidget
{
	Q_OBJECT

public:		// Consruction & Destruction
	QextMdiChildView( const QString& name, QWidget* parentWidget = 0);
	~QextMdiChildView();
protected:		// Fields
	QString     m_szCaption;
	QWidget*    m_focusedChildWidget;
	QWidget*    m_firstFocusableChildWidget;
	QWidget*    m_lastFocusableChildWidget;

public:     // Methods
	/**
	 * Memorize first focusable child widget <br>
	 */
   void setFirstFocusableChildWidget(QWidget*);
	/**
	 * Memorize last focusable child widget <br>
	 */
   void setLastFocusableChildWidget(QWidget*);
	/**
	 * Returns current focused child widget <br>
	 */
   QWidget* focusedChildWidget();
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
	virtual void setWindowCaption(const QString& szCaption);
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
	 */
	virtual QPixmap * myIconPtr();
	/**
	 * Must be overridden in child classes.
	 */
//F.B.	virtual void applyOptions();
	/**
	 * Minimizes this window when it is attached to the Mdi manager.<br>
	 * Otherwise has no effect
	 */
	virtual void minimize(bool bAnimate);   //Useful only when attached
	/**
	 * Maximizes this window when it is attached to the Mdi manager.<br>
	 * Otherwise has no effect
	 */
   virtual void maximize(bool bAnimate);   //Useful only when attached
   /**
     * Interpose in event loop of all current child widgets.
     * Must be recalled after dynamic adding of new child widgets!
     */
   void installEventFilterForAllChildren();
   /**
     * Switches interposing in event loop of all current child widgets off.
     */
   void removeEventFilterForAllChildren();
	
//F.B.	virtual void setProperties(QextMdiChildViewProperty * p); //do nothing here....
//F.B.	virtual void saveProperties();
public slots:
	/**
	 * Attaches this window to the Mdi manager.<br>
	 * It calls the QextMdiMainFrm attachWindow function , so if you have a pointer<br>
	 * to this QextMdiMainFrm you'll be faster calling that function.<br>
	 * Useful as slot.
	 */
	virtual void attach();
	/**
	 * Detaches this window from the Mdi manager.<br>
	 * It calls the QextMdiMainFrm detachWindow function , so if you have a pointer<br>
	 * to this QextMdiMainFrm you'll be faster calling that function.<br>
	 * Useful as slot.
	 */
	virtual void detach();
	
	virtual void minimize(); //Overload and slot
	virtual void maximize(); //Overload and slot
	/**
	 * Restores this window when it is attached to the Mdi manager.
	 */
	virtual void restore();    //Useful only when attached
	virtual void youAreAttached(QextMdiChildFrm *lpC);
	virtual void youAreDetached();
 protected:	// Protected methods
	/**
	 * Ignores the event and calls QextMdiMainFrm::childWindowCloseRequest
	 * @see QextMdiMainFrm::childWindowCloseRequest
	 */
	virtual void closeEvent(QCloseEvent *e);
   virtual bool eventFilter(QObject *obj, QEvent *e);
   virtual void focusInEvent(QFocusEvent *);
	
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
