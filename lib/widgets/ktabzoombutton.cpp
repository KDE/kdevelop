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

#include <qpainter.h>
#include <qsizepolicy.h>
#include <qstyle.h>


#include "ktabzoombutton.h"


class KTabZoomButtonPrivate
{
public:

  KTabZoomPosition::Position m_position;

};


KTabZoomButton::KTabZoomButton(const QString &text, QWidget *parent, KTabZoomPosition::Position pos, const char *name)
  : QPushButton(text, parent, name)
{
  d = new KTabZoomButtonPrivate;
  
  d->m_position = pos;

  setFlat(true);
  setToggleButton(true);

  setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
}


KTabZoomButton::~KTabZoomButton()
{
  delete d;
}


void KTabZoomButton::drawButton(QPainter *p)
{
  int w = fontMetrics().width(text()) + 2*fontMetrics().width('m');
  int h = fontMetrics().height()+2;

  QPixmap pixmap(w, h);
  QPainter painter(&pixmap);
  
  pixmap.fill(eraseColor());
  
  QStyle::SFlags flags = QStyle::Style_Default;
  if (isEnabled())
    flags |= QStyle::Style_Enabled;
  if (isDown())
    flags |= QStyle::Style_Down;
  if (isOn())
    flags |= QStyle::Style_On;
  if (! isFlat() && ! isDown())
    flags |= QStyle::Style_Raised;

  style().drawControl(QStyle::CE_PushButton, &painter, this, QRect(0,0,w,h), colorGroup(), flags);
  style().drawControl(QStyle::CE_PushButtonLabel, &painter, this, QRect(0,0,w,h), colorGroup(), flags);

  if (hasFocus())
    style().drawPrimitive(QStyle::PE_FocusRect, &painter, QRect(1,1,w-2,h-2), colorGroup(), flags);
  
  switch (d->m_position)
  {
  case KTabZoomPosition::Top:
  case KTabZoomPosition::Bottom:
    p->drawPixmap(0,0, pixmap);
    break;

  case KTabZoomPosition::Left:
    p->rotate(-90);
    p->drawPixmap(-height(), 0, pixmap);
    break;

  case KTabZoomPosition::Right:
    p->rotate(90);
    p->drawPixmap(0,-width(), pixmap);
    break;
  }		   
}


void KTabZoomButton::drawButtonLabel(QPainter *p)
{
  drawButton(p);
}


QSize KTabZoomButton::sizeHint () const
{
  int w = fontMetrics().width(text()) + 2*fontMetrics().width('m');

  if (d->m_position == KTabZoomPosition::Top || d->m_position == KTabZoomPosition::Bottom)
    return QSize(w, fontMetrics().height()+2);

  return QSize(fontMetrics().height()+2, w);
}

#include "ktabzoombutton.moc"
