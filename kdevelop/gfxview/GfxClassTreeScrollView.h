/***************************************************************************
                file       : GfxClassTreeScrollView.h
 ---------------------------------------------------------------------------
               begin       : Jun 10 1999
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
#ifndef GFXCLASSTREESCROLLVIEW_H_INCLUDED
#define GFXCLASSTREESCROLLVIEW_H_INCLUDED


#include <qscrollview.h>
#include "GfxClassTree.h"
#include "../sourceinfo/classstore.h"




class CGfxClassTreeScrollView : public QScrollView
{
  Q_OBJECT

 public:
  CGfxClassTree *m_classtree;

 public:
  /** Constructor */
  CGfxClassTreeScrollView(QWidget *aparentwidget);

  /** Destructor */
  ~CGfxClassTreeScrollView();

  /** implementation of resize event */
  virtual void resizeEvent(QResizeEvent *resevent);

};


#endif
