/***************************************************************************
                          mdiview.cpp  -  description
                             -------------------
    begin                : Thu Jul 27 2000
    copyright            : (C) 2000 by Ralf Nolden
    email                : Ralf.Nolden@post.rwth-aachen.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qlayout.h>

#include "mdiview.h"

MdiView::MdiView( QWidget* pClient, const QString& caption, QWidget* pParent, const char *name, WFlags f)
: QextMdiChildView( caption, pParent, name, f)
{
  pClient->reparent( this, 0, QPoint(0,0));

  QHBoxLayout* hbl = new QHBoxLayout( this);
  hbl->addWidget( pClient);
}

MdiView::~MdiView()
{
}
