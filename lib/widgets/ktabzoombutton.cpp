#include <qpainter.h>
#include <qsizepolicy.h>


#include "ktabzoombutton.h"


class KTabZoomButtonPrivate
{
public:

  KTabZoomPosition::Position m_position;

};


KTabZoomButton::KTabZoomButton(const QString &text, QWidget *parent, KTabZoomPosition::Position pos, const char *name)
  : QToolButton(parent, name)
{
  d = new KTabZoomButtonPrivate;
  
  d->m_position = pos;
  
  setAutoRaise(true);

  setUsesTextLabel(true);
  setTextLabel(text);
  setToggleButton(true);
  
  setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
}


KTabZoomButton::~KTabZoomButton()
{
  delete d;
}


void KTabZoomButton::drawButtonLabel(QPainter *p)
{
  switch (d->m_position)
  {
  case KTabZoomPosition::Top:
  case KTabZoomPosition::Bottom:
    QToolButton::drawButtonLabel(p);
    break;

  case KTabZoomPosition::Left:
    p->rotate(270.0);
    p->drawText(-height(), 0, height(), width(), Qt::AlignCenter, textLabel());
    break;

  case KTabZoomPosition::Right:
    p->rotate(90);
    p->drawText(0, -width(), height(), width(), Qt::AlignCenter, textLabel());
    break;
  }
}


QSize KTabZoomButton::sizeHint () const
{
  int w = fontMetrics().width(textLabel()) + 2*fontMetrics().width('m');

  if (d->m_position == KTabZoomPosition::Top || d->m_position == KTabZoomPosition::Bottom)
    return QSize(w, fontMetrics().height()+2);

  return QSize(fontMetrics().height()+2, w);
}



#include "ktabzoombutton.moc"
