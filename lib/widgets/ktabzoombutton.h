/***************************************************************************
 *   Copyright (C) 2001-2003                                               *
 *   The KDevelop Team                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __K_TABZOOMBUTTON_H__
#define __K_TABZOOMBUTTON_H__

#include <qpushbutton.h>

#include "ktabzoomposition.h"


class KTabZoomButtonPrivate;

class KTabZoomButton : public QPushButton
{
  Q_OBJECT

public:

  KTabZoomButton(const QString &text, QWidget *parent, KTabZoomPosition::Position pos, const char *name=0);
  ~KTabZoomButton();


protected:

  virtual void drawButtonLabel(QPainter *);
  virtual void drawButton(QPainter *);
  virtual QSize sizeHint() const;


private:

  KTabZoomButtonPrivate *d;

};

#endif
