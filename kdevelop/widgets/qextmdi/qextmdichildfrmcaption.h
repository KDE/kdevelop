//----------------------------------------------------------------------------
//    filename             : qextmdichildfrmcaption.h
//----------------------------------------------------------------------------
//    Project              : Qt MDI extension
//
//    begin                : 07/1999       by Szymon Stefanek as part of kvirc
//                                         (an IRC application)
//    changes              : 09/1999       by Falk Brettschneider to create an
//                           - 06/2000     stand-alone Qt extension set of
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

#ifndef _QEXTMDICAPTION_H_
#define _QEXTMDICAPTION_H_

#include <qwidget.h>

#include "qextmdidefines.h"

class DLL_IMP_EXP_QEXTMDICLASS QextMdiChildFrm;

/**
  * @short Internal class.
  * It's the caption bar of a child frame widget.
  */

class DLL_IMP_EXP_QEXTMDICLASS QextMdiChildFrmCaption : public QWidget
{
   Q_OBJECT
public:
   QextMdiChildFrmCaption(QextMdiChildFrm *parent);
   ~QextMdiChildFrmCaption();
   void setActive(bool bActive);
   void setCaption(const QString& text);
   int heightHint();
public slots:
   void slot_moveViaSystemMenu();

protected:
   virtual void paintEvent(QPaintEvent *e);
   virtual void mousePressEvent(QMouseEvent *);
   virtual void mouseDoubleClickEvent(QMouseEvent *);
   virtual void mouseReleaseEvent(QMouseEvent *);
   virtual void mouseMoveEvent(QMouseEvent *e);
   
// attributes
public:
   QString           m_szCaption;

protected: // Protected attributes
   QextMdiChildFrm  *m_pParent;
   bool              m_bActive;
   bool              m_bCanMove;
   QPoint            m_offset;
};

#endif //_QEXTMDICAPTION_H_
