/***************************************************************************
         mdiview.h  -  a special QextMdiChildView covering any QWidget
                             -------------------
    begin                : Thu Jul 27 2000
    copyright            : (C) 2000 by Falk Brettschneider
    email                : <Falk Brettschneider> falk@kdevelop.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _MDIVIEW_H_
#define _MDIVIEW_H_

#include <qextmdichildview.h>

class MdiView : public QextMdiChildView
{
  Q_OBJECT

public:
  MdiView( QWidget* pClient, const QString& caption, QWidget* pParent=0L, const char *name=0L, WFlags f=0);
  ~MdiView();
};

#endif  // _MDIVIEW_H_
