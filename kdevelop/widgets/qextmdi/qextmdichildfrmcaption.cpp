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

#include <qpainter.h>
#include <qapplication.h>

#include "qextmdidefines.h"
#include "qextmdichildfrmcaption.h"
#include "qextmdichildfrm.h"
#include "qextmdichildarea.h"

//////////////////////////////////////////////////////////////////////////////
// Class   : QextMdiChildFrmCaption
// Purpose : An MDI label that draws the title
//
//
//////////////////////////////////////////////////////////////////////////////

//============== QextMdiChildFrmCaption =============//

QextMdiChildFrmCaption::QextMdiChildFrmCaption(QextMdiChildFrm *parent)
:QWidget(parent, "qextmdi_childfrmcaption")
{
	m_szCaption    = tr("Unnamed");
	m_bActive      = false;
	m_bCanMove     = false;
	m_pParent      = parent;
	setBackgroundMode(NoBackground);
	setFocusPolicy(NoFocus);
}

//============== ~QextMdiChildFrmCaption =============//

QextMdiChildFrmCaption::~QextMdiChildFrmCaption()
{
}

//============= mousePressEvent ==============//

void QextMdiChildFrmCaption::mousePressEvent(QMouseEvent *e)
{
   grabMouse();
   m_relativeMousePos = e->pos() - pos() + QPoint(QEXTMDI_MDI_CHILDFRM_BORDER,QEXTMDI_MDI_CHILDFRM_BORDER);
	//F.B. QApplication::setOverrideCursor(Qt::sizeAllCursor,true);
	m_bCanMove=true;
}

//============= mouseReleaseEvent ============//

void QextMdiChildFrmCaption::mouseReleaseEvent(QMouseEvent *)
{
	m_bCanMove=false;
	QApplication::restoreOverrideCursor();
	releaseMouse();
}

//============== mouseMoveEvent =============//
void QextMdiChildFrmCaption::mouseMoveEvent(QMouseEvent *e)
{
	if(m_bCanMove){
		QPoint diff = (e->pos() - m_relativeMousePos);
		if(e->state() & LeftButton)m_pParent->moveWindow( diff, m_relativeMousePos);
	}
}

//=============== setActive ===============//

void QextMdiChildFrmCaption::setActive(bool bActive)
{
	if(m_bActive==bActive)return;
	m_bActive=bActive;
	repaint(false);
}

//=============== setCaption ===============//

void QextMdiChildFrmCaption::setCaption(const QString& text)
{
	m_szCaption=text;
	repaint(false);
}

//============== heightHint ===============//

int QextMdiChildFrmCaption::heightHint()
{
	int hght=m_pParent->m_pManager->m_captionFontLineSpacing+2;
#ifdef WIN32
	if(hght<18)hght=18;
#else // in case of Unix: KDE look
	if(hght<20)hght=20;
#endif
	return hght;
}

//=============== paintEvent ==============//

void QextMdiChildFrmCaption::paintEvent(QPaintEvent *)
{
	QPainter p(this);
	QRect r=rect();
	p.setFont(m_pParent->m_pManager->m_captionFont);
	if(m_bActive){
		p.fillRect(r,m_pParent->m_pManager->m_captionActiveBackColor);
		p.setPen(m_pParent->m_pManager->m_captionActiveForeColor);
	} else {
		p.fillRect(r,m_pParent->m_pManager->m_captionInactiveBackColor);
		p.setPen(m_pParent->m_pManager->m_captionInactiveForeColor);
	}
#ifdef WIN32
	r.setLeft(r.left()+19); //Shift the text after the icon
#else // in case of Unix: KDE look
	r.setLeft(r.left()+22); //Shift the text after the icon
#endif
	p.drawText(r,AlignVCenter|AlignLeft|SingleLine,m_szCaption);
	
}

//============= mouseDoubleClickEvent ===========//

void QextMdiChildFrmCaption::mouseDoubleClickEvent(QMouseEvent *)
{
	m_pParent->maximizePressed();
}
