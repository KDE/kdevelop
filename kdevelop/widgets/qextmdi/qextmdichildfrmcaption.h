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

#ifndef _QEXTMDICAPTION_H_
#define _QEXTMDICAPTION_H_

#include <qwidget.h>
#include <qpixmap.h>

#include "qextmdidefines.h"

class DLL_IMP_EXP_QEXTMDICLASS QextMdiChildFrm;

/**
* Internal class
* @internal
*/

class QextMdiChildFrmCaption : public QWidget
{
	Q_OBJECT
public:
	QextMdiChildFrmCaption(QextMdiChildFrm *parent);
	~QextMdiChildFrmCaption();
public:
	QString m_szCaption;
	bool m_bActive;
	bool m_bCanMove;
	QextMdiChildFrm *m_pParent;
	QPoint m_relativeMousePos;
	/**
	* This is a POINTER to an icon 16x16. If this is 0 no icon is painted.
	*/
	QPixmap *m_pIcon;
public:
	void setActive(bool bActive);
	void setCaption(const QString& text);
	int heightHint();
protected:
	virtual void paintEvent(QPaintEvent *e);
	virtual void mousePressEvent(QMouseEvent *);
	virtual void mouseDoubleClickEvent(QMouseEvent *);
	virtual void mouseReleaseEvent(QMouseEvent *);
	virtual void mouseMoveEvent(QMouseEvent *e);
};



#endif //_QEXTMDICAPTION_H_
