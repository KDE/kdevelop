/***************************************************************************
		file       : GfxClassTreeWindow.h
 ---------------------------------------------------------------------------
               begin       : Jun 12 1999
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

#ifndef GFXCLASSTREEWINDOW_H_INCLUDED
#define GFXCLASSTREEWINDOW_H_INCLUDED


#include <qlist.h>
#include <qwidget.h>
#include <qpushbutton.h>
#include <qcombobox.h>
#include "GfxClassTreeScrollView.h"




class CGfxClassTreeWindow : public QWidget
{
  Q_OBJECT

 public:
  QPushButton *m_foldbtn;
  QPushButton *m_unfoldbtn;
  QPushButton *m_printbtn;
  QComboBox *m_classcombo;
  CGfxClassTreeScrollView *m_treescrollview;

  /** Constructor */
  CGfxClassTreeWindow(QWidget *aparent);

  /** Destructor */
  ~CGfxClassTreeWindow();

  /** (re-)Initialize the class tree */
  void InitializeTree(QList<ClassTreeNode> *forest);

  /** Implementation of resizeEvent() */
  virtual void resizeEvent(QResizeEvent *resevent);

  signals:
   void gotoClassDefinition(ParsedClass *);

  public slots:

  /** Called when the "Fold all" button is clicked */
  void foldClicked();

  /** Called when the "Unfold all" button is clicked */
  void unfoldClicked();

  /** Called when a combobox item is selected */
  void itemSelected(int index);

  /** Called when print button is clicked */
  void printClicked();

  /** Called when a signal to go to the definition from a box
      was emitted */
  void slotGotoClassDefinition(ParsedClass *pClass);
};

#endif
