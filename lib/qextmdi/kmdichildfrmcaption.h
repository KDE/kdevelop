//----------------------------------------------------------------------------
//    filename             : kmdichildfrmcaption.h
//----------------------------------------------------------------------------
//    Project              : KDE MDI extension
//
//    begin                : 07/1999       by Szymon Stefanek as part of kvirc
//                                         (an IRC application)
//    changes              : 09/1999       by Falk Brettschneider to create an
//                           - 06/2000     stand-alone Qt extension set of
//                                         classes and a Qt-based library
//                           2000-2003     maintained by the KDevelop project
//
//    copyright            : (C) 1999-2003 by Falk Brettschneider
//                                         and
//                                         Szymon Stefanek (stefanek@tin.it)
//    email                :  falkbr@kdevelop.org (Falk Brettschneider)
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
#ifndef _KMDI_CHILD_FRM_CAPTION_H_
#define _KMDI_CHILD_FRM_CAPTION_H_

#include <qwidget.h>

#include "kmdidefines.h"

class KMdiChildFrm;


class KMdiChildFrmCaptionPrivate;

/**
* @short Internal class.
*
* It's the caption bar of a child frame widget.
*/
class KMDI_EXPORT KMdiChildFrmCaption : public QWidget
{
   Q_OBJECT
public:
   /**
   * Constructor
   */
   KMdiChildFrmCaption(KMdiChildFrm *parent);
   /**
   * Destructor
   */
   ~KMdiChildFrmCaption();
   /**
   * Repaint the caption bar in active background colors
   */
   void setActive(bool bActive);
   /**
   * Repaint with a new caption bar title
   */
   void setCaption(const QString& text);
   /**
   * Returns the caption bar height depending on the used font
   */
   int heightHint();

public slots:
   /**
   * Grabs the mouse, a move cursor, sets a move indicator variable to true and keeps the global mouse position in mind
   */
   void slot_moveViaSystemMenu();

protected:
   /**
   * Draws the caption bar and its title using the settings
   */
   virtual void paintEvent(QPaintEvent *e);
   /**
   * The same as KMdiChildFrmCaption::slot_moveViaSystemMenu
   */
   virtual void mousePressEvent(QMouseEvent *);
   /**
   * Calls maximizePressed of the parent widget ( KMdiChildFrm )
   */
   virtual void mouseDoubleClickEvent(QMouseEvent *);
   /**
   * Restore the normal mouse cursor, set the state variable back to 'not moving'
   */
   virtual void mouseReleaseEvent(QMouseEvent *);
   /**
   * Checks if out of move range of the KMdiChildArea and calls KMdiChildFrm::move
   */
   virtual void mouseMoveEvent(QMouseEvent *e);
   /**
   * Computes a new abbreviated string from a given string depending on a given maximum width
   */
   QString abbreviateText(QString origStr, int maxWidth);

// attributes
public:
   /**
   * the title string shown in the caption bar
   */
   QString           m_szCaption;

protected: // Protected attributes
   /**
   * parent widget
   */
   KMdiChildFrm  *m_pParent;
   /**
   * state variable indicating whether activated or not activated
   */
   bool              m_bActive;
   /**
   * the position offset related to its parent widget (internally used for translating mouse move positions
   */
   QPoint            m_offset;
   /**
   * True if the child knows that it is currently being dragged.
   */
   bool              m_bChildInDrag;

private:
   KMdiChildFrmCaptionPrivate *d;
};

#endif //_KMDICAPTION_H_

// kate: space-indent on; indent-width 2; replace-tabs on;
