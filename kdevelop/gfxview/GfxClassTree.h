/***************************************************************************
                file       : GfxClassTree.h
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
#ifndef GFXCLASSTREE_H_INCLUDED
#define GFXCLASSTREE_H_INCLUDED

#define PRINTTREE_YOFFSET 15

#include <qwidget.h>
#include <qprinter.h>
#include <qptrlist.h>
#include "GfxClassBox.h"
#include "../sourceinfo/classstore.h"

class CGfxClassTree : public QWidget
{
  Q_OBJECT

 public: // Constructor & Destructor

  /** Constructor */
  CGfxClassTree(QWidget *aparentwidget);

  /** Destructor */
  ~CGfxClassTree();

  /*===================================================*/

  QPtrList<CGfxClassBox> m_boxlist;

 private: // Private methods
  /** Insert a box object in the class box tree */
  void InsertBox(CGfxClassBox *abox,
		 CGfxClassBox *aparentbox,
		 CGfxClassBox *asiblingbox);

 public: // Public methods

  /** Remove all class boxes in the tree */
  void RemoveAll();

  /** Get CGfxClassBox object on box id */
  CGfxClassBox *GetBoxId(int boxid);

  /** Get class subtree nodes in a list */
  QPtrList<CGfxClassBox> *getSubtree(CGfxClassBox *abox);

  /** Insert a class tree */
  CGfxClassBox *InsertClassTree(CGfxClassBox *baseclassbox,
				CGfxClassBox *sibclassbox,
				ClassTreeNode *atreenode);

  /** Insert a class forest */
  void InsertClassForest(CGfxClassBox *baseclassbox,
			 QPtrList<ClassTreeNode> *forest);

  /** Refresh the entire class tree view */
  void RefreshClassForest(QPtrList<ClassTreeNode> *forest);

  /** Refresh tree widget size */
  void RefreshTreeSize();

  /** Fold or unfold all class boxes */
  void SetUnfoldAll(bool unfolded);

  /** Print class tree */
  void onPrintTree( QPrinter *pr, QPtrList<CGfxClassBox> *boxlist );

  /** Implementation of resizeEvent() */
  virtual void resizeEvent(QResizeEvent *);

  /** Implementation of paintEvent() */
  virtual void paintEvent(QPaintEvent *);

 signals:
  void PosRefresh(int);

 public slots:
  void stateChange(CGfxClassBox *abox);
  void drawConnection(CGfxClassBox *abox);
  void slotPrintSubTree(CGfxClassBox *abox);

};


#endif
