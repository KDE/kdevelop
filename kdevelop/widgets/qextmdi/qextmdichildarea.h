//----------------------------------------------------------------------------
//    filename             : qextmdichildarea.h
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

#ifndef _QEXTMDICHILDAREA_H_
#define _QEXTMDICHILDAREA_H_

#include <qframe.h>

#include "qextmdichildfrm.h"
#include "qextmdichildview.h"

/**
  * @short Internal class.
  * The widget where the child frames live in. Furthermore the manager for those frame widgets.
  */
class DLL_IMP_EXP_QEXTMDICLASS QextMdiChildArea : public QFrame
{
	friend class QextMdiChildFrmCaption;
	friend class QextMdiChildFrm;
	Q_OBJECT
public:		// Consruction & Destruction
	QextMdiChildArea(QWidget *parent);
	/**
	* Destructor : THERE should be no child windows anymore...
	* Howewer it simply deletes all the child widgets :)
	*/
	~QextMdiChildArea();
public:		// Fields
	/**
	* Z Order stack (top=last)
	*/
	QList<QextMdiChildFrm> *m_pZ; //Auto delete enabled
private:
	QFont m_captionFont;
	QColor m_captionActiveBackColor;
	QColor m_captionActiveForeColor;
	QColor m_captionInactiveBackColor;
	QColor m_captionInactiveForeColor;
	int m_captionFontLineSpacing;
public:		// Methods
	/**
	* Appends a new QextMdiChildFrm to this manager.
	* The child is shown,raised and gets focus if this window has it.
	*/
	void manageChild(QextMdiChildFrm *lpC,bool bShow=TRUE,bool bCascade=TRUE);
	/**
	* Destroys a QextMdiChildFrm managed.<br>
	* Note that if a client is attached to this child , it is deleted too!
	*/
	void destroyChild(QextMdiChildFrm *lpC,bool bFocusTopChild = TRUE);
	/**
	* Destroys a QextMdiChildFrm managed.<br>
	* Note that if a client is attached to this child , it is NOT deleted!
	*/
	void destroyChildButNotItsView(QextMdiChildFrm *lpC,bool bFocusTopChild = TRUE);
	/**
	* Brings the child lpC to the top of the stack
	* The children is focused if bSetFocus is TRUE
	* otherwise is raised only
	*/
	void setTopChild(QextMdiChildFrm *lpC,bool bSetFocus=FALSE);
	/**
	* Returns the topmost child (the active one) or 0 if there are no children.
	* Note that the topmost child may be also hidded , if ALL the windows are minimized.
	*/
	inline QextMdiChildFrm * topChild(){ return m_pZ->last(); };
	/**
	* Returns the number of visible children
	*/
	int getVisibleChildCount();

	void setMdiCaptionFont(const QFont &fnt);
	void setMdiCaptionActiveForeColor(const QColor &clr);
	void setMdiCaptionActiveBackColor(const QColor &clr);
	void setMdiCaptionInactiveForeColor(const QColor &clr);
	void setMdiCaptionInactiveBackColor(const QColor &clr);
public slots:
	/**
	* Cascades the windows resizing it to the minimum size.
	*/
	void cascadeWindows();
	/**
	* Casecades the windows resizing it to the maximum available size.
	*/
	void cascadeMaximized();
	void expandVertical();
	void expandHorizontal();
	/**
	* Foces focus to the topmost child
	* In case that it not gets focused automatically...
	* Btw : It should not happen.
	*/
	void focusTopChild();
	/**
	* Tile Pragma
	*/
	void tilePragma();
	/**
	* Tile Anodine
	*/
	void tileAnodine();
	/**
	* Tile Vertically
	*/
   void tileVertically();
	/**
	* positioning of minimized child frames
	*/
   void layoutMinimizedChildren();
	
protected:	// Protected methods
	void tileAllInternal(int maxWnds);
	virtual void focusInEvent(QFocusEvent *);
	virtual void resizeEvent(QResizeEvent *);
	QPoint getCascadePoint(int indexOfWindow);
	void mousePressEvent(QMouseEvent *e);
	void childMinimized(QextMdiChildFrm *lpC,bool bWasMaximized);
	
	void undockWindow(QWidget *lpC);

signals: // Signals
   void topChildChanged(QextMdiChildView*);
   /** signalizes that the child frames are no longer maximized */
   void noLongerMaximized(QextMdiChildFrm*);
   /** signalizes that the child frames are maximized now*/
   void nowMaximized();
   void sysButtonConnectionsMustChange(QextMdiChildFrm*, QextMdiChildFrm*);
   void popupWindowMenu( QPoint);
};

#endif   // _QEXTMDICHILDAREA_H_
