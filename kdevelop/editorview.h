/***************************************************************************
              editorview.h  - 
                             -------------------

     begin                : 20 Oct 1999
     copyright            : (C) 1999 by Sandy Meier,(C) the KDevelop Team
     email                : smeier@rz.uni-potsdam.de
  ***************************************************************************/

 /***************************************************************************
  *                                                                         *
  *   This program is free software; you can redistribute it and/or modify  *
  *   it under the terms of the GNU General Public License as published by  *
  *   the Free Software Foundation; either version 2 of the License, or     *
  *   (at your option) any later version.                                   *
  *                                                                         *
  ***************************************************************************/

#ifndef EDITORVIEW_H
#define EDITORVIEW_H

#include "component.h"
#include "./widgets/qextmdi/qextmdichildview.h"
#include <kapp.h>
#include <qdatetime.h>
#include <qsplitter.h>
#include "ceditwidget.h"

class CEditWidget;
/** An abstraction layer for the editwidget.
  *@author Sandy Meier
  */
class EditorView : public QextMdiChildView, public Component {
  Q_OBJECT

public: // Constructor and destructor

  EditorView (QWidget* parent=0,const char* name=0);
  CEditWidget* editorfirstview;
  QSplitter* split;
  CEditWidget* editorsecondview;
  
  CEditWidget* currentEditor();

protected slots:
  void slotSplitHorizontal();
  void slotSplitVertical();
  void slotUnSplit();

 protected:
  virtual void resizeEvent (QResizeEvent *e); 
  virtual void closeEvent(QCloseEvent* e);
 signals:
  void closing(EditorView* editorview);

  
};

#endif


