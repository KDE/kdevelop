/***************************************************************************
		file       : GfxClassBox.h
 ---------------------------------------------------------------------------
               begin       : Jun 7 1999
               copyright   : (C) 1999 by Jörgen Olsson
               email       : jorgen@cenacle.net
 ***************************************************************************/


/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef GFXCLASSBOX_H_INCLUDED
#define GFXCLASSBOX_H_INCLUDED

#include <qwidget.h>
#include <qpushbutton.h>
#include <qprinter.h>
#include <qpixmap.h>
#include "../classparser/ParsedClass.h"

// Colour of boxes
#define CLASSBOXCOL_INSYSTEM          QColor(0xd0,0xff,0xd0)
#define CLASSBOXCOL_NOTINSYSTEM       QColor(0xd0,0xd0,0xff)
#define PRINT_CLASSBOXCOL_INSYSTEM    QColor(0xff,0xff,0xff)
#define PRINT_CLASSBOXCOL_NOTINSYSTEM QColor(0xd0,0xd0,0xd0)

// Size of boxes
// #define CLASSBOXWIDTH 100
#define CLASSBOXHEIGHT 20

// Space between boxes
#define CLASSBOX_XSPACE 5
#define CLASSBOX_YSPACE 5

// Connections between boxes
#define CONN_CHILD_DELTA_STARTX 0
#define CONN_CHILD_DELTA_STARTY 10
#define CONN_CHILD_DELTA_STOPX  -10
#define CONN_CHILD_DELTA_STOPY  10


class CGfxClassBox;
class CGfxClassBox : public QWidget
{
  Q_OBJECT

    // private // Members
    public:
  CParsedClass *m_class;     // The class that this box represents
  CGfxClassBox *m_parent;    // The parent box of this box (Base class)
  CGfxClassBox *m_sibling;   // The sibling box of this box 
  CGfxClassBox *m_prevnode;  // Box with boxid = m_boxid - 1

  QPushButton *m_foldbtn;        // Button for fold/unfold subtree
  bool m_unfolded;           // True=show all subclasses under this class
  int m_boxwidth;            // The width of the box

 public: // Members
  int m_boxid;               // Box id (internal use, ignore it!)
  QPixmap *plusbtn;
  QPixmap *minusbtn;
  QString m_name;

 public: 
  /** Constructor */
  CGfxClassBox(CParsedClass *aclass,
	       QString *aname,
	       QWidget *aparentwidget);  

  /** Destructor */
  ~CGfxClassBox();

  /** Set parent class box (CGfxClassBox representing the base class) */
  void SetParentClassBox(CGfxClassBox *aclassbox);

  /** Set sibling class box */
  void SetSiblingClassBox(CGfxClassBox *aclassbox);

  /** Set previous class box */
  void SetPreviousClassBox(CGfxClassBox *aclassbox);

  /** Set name */
  void setName(QString *name);

  /** Change folded/unfolded status */
  void SetUnfolded(bool unfolded);

  /** Unfold all ancestors */
  void MakeVisible(bool no_self_unfold);

  /** Show fold/unfold button */
  void ShowFoldButton();

   /** Returns TRUE if subclass CGfxClassBoxes is to be shown */
  bool IsUnfolded();

  /** Returns TRUE if baseclass CGfxClassBox (m_parent) is unfolded */
  bool IsVisible();

  /** Return TRUE if this is an ancestor to abox */
  bool hasAncestor(CGfxClassBox *abox);

  /** Get Y top position */
  int GetY();

  /** Get X leftmost position */
  int GetX();

  /** Get Y bottom position */
  int GetYDepth();

  /** Get X rightmost position */
  int GetXDepth();

 protected:
  /** Implementation of paintEvent */
  void paintEvent( QPaintEvent * );

  /** Implementation of mousePressEvent */
  void mousePressEvent ( QMouseEvent *mouseevent ); 

  /* ============================================================== */

 signals:
  void stateChange(CGfxClassBox *);
  void drawConnection(CGfxClassBox *);
  void gotoClassDefinition(CParsedClass *);
  void PrintSubTree(CGfxClassBox *);

 public slots:
  void PosRefresh(int boxid); 
  void btnClicked();
  void slotGotoDefinition();
  void slotPrintSubTree();

  
};

 
#endif
