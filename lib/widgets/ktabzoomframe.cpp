#include <qlayout.h>
#include <qlabel.h>
#include <qwidgetstack.h>
#include <qtoolbutton.h>
#include <qptrlist.h>
#include <qframe.h>
#include <qtimer.h>
#include <qiconset.h>


#include <kglobalsettings.h>
#include <kdebug.h>
#include <kapplication.h>


#include "sticky.xpm"


#include "ktabzoomframe.h"


class KTZFWidgetInfo
{
public:

  QWidget *m_widget;
  int     m_index;
  QString m_title;

};


class KTabZoomFramePrivate
{
public:

  KTabZoomPosition::Position m_position;
  QLabel                     *m_title;
  QWidgetStack               *m_stack;
  int                        m_count;
  QPtrList<KTZFWidgetInfo>   m_info;
  QFrame                     *m_slider;
  bool                       m_sliding;
  QPoint                     m_slideStart;
  int                        m_initialPos, m_initialSize;
  QToolButton		     *m_closeButton, *m_dockButton;

};


KTabZoomFrame::KTabZoomFrame(QWidget *parent, KTabZoomPosition::Position pos, const char *name)
  : QWidget(parent, name)
{
  d = new KTabZoomFramePrivate;
  d->m_info.setAutoDelete(true);

  d->m_position = pos;

  QFrame *frame = new QFrame(this);

  frame->setFrameStyle(QFrame::Box | QFrame::Plain);

  QVBoxLayout *vbox = new QVBoxLayout(frame);
  vbox->setMargin(2);

  QFrame *f = new QFrame(frame);
  f->setBackgroundColor(KGlobalSettings::activeTitleColor());
  vbox->addWidget(f);

  QHBoxLayout *hbox = new QHBoxLayout(f);
  if (pos == KTabZoomPosition::Right)
    hbox->setDirection(QBoxLayout::RightToLeft);

  d->m_title = new QLabel(f);
  d->m_title->setPaletteBackgroundColor(KGlobalSettings::activeTitleColor());
  d->m_title->setPaletteForegroundColor(KGlobalSettings::activeTextColor());
  d->m_title->setAlignment(Qt::AlignCenter | Qt::ShowPrefix);
  hbox->addWidget(d->m_title, 1);

  QToolButton::ArrowType arrow = QToolButton::LeftArrow;
  switch (pos)
  {
  case KTabZoomPosition::Right:
    arrow = QToolButton::RightArrow;
    break;
  case KTabZoomPosition::Bottom:
    arrow = QToolButton::DownArrow;
    break;
  case KTabZoomPosition::Top:
    arrow = QToolButton::UpArrow;
    break;
  default:
    break;
  }

  d->m_dockButton = new QToolButton(f);
  d->m_dockButton->setPixmap(QPixmap(sticky));
  d->m_dockButton->setFixedSize(12,12);
  d->m_dockButton->setToggleButton(true);

  QPalette pal = d->m_dockButton->palette();
  QColorGroup cg = pal.active();
  cg.setColor(QColorGroup::Background, KGlobalSettings::activeTitleColor());
  pal.setActive(cg) ;
  cg = pal.inactive();
  cg.setColor(QColorGroup::Background, KGlobalSettings::activeTitleColor());
  pal.setInactive(cg) ;
  cg = pal.disabled();
  cg.setColor(QColorGroup::Background, KGlobalSettings::activeTitleColor());
  pal.setDisabled(cg);
  d->m_dockButton->setPalette(pal);

  hbox->addWidget(d->m_dockButton);

  connect(d->m_dockButton, SIGNAL(toggled(bool)), this, SLOT(slotDockButtonToggled(bool)));

  hbox->addSpacing(4);

  d->m_closeButton = new QToolButton(arrow, f);
  d->m_closeButton->setFixedSize(12,12);
  d->m_closeButton->setPalette(pal);

  hbox->addWidget(d->m_closeButton);

  connect(d->m_closeButton, SIGNAL(clicked()), this, SLOT(slotCloseButtonClicked()));

  hbox->addSpacing(4);

  d->m_stack = new QWidgetStack(frame);
  d->m_stack->setMinimumHeight(100);
  vbox->addWidget(d->m_stack, 1);

  QBoxLayout *box = 0;
  d->m_slider = new QFrame(this);
  d->m_slider->setFrameStyle(QFrame::Panel | QFrame::Raised);

  switch (pos)
  {
  case KTabZoomPosition::Left:
    box = new QHBoxLayout(this);
    box->addWidget(frame,1);
    box->addWidget(d->m_slider);
    d->m_slider->setFixedWidth(4);
    d->m_slider->setCursor(Qt::sizeHorCursor);
    break;

  case KTabZoomPosition::Right:
    box = new QHBoxLayout(this);
    box->addWidget(d->m_slider);
    box->addWidget(frame,1);
    d->m_slider->setFixedWidth(4);
    d->m_slider->setCursor(Qt::sizeHorCursor);
    break;

  case KTabZoomPosition::Bottom:
    box = new QVBoxLayout(this);
    box->addWidget(d->m_slider);
    box->addWidget(frame,1);
    d->m_slider->setFixedHeight(4);
    d->m_slider->setCursor(Qt::sizeVerCursor);
    break;

  case KTabZoomPosition::Top:
    box = new QVBoxLayout(this);
    box->addWidget(frame,1);
    box->addWidget(d->m_slider);
    d->m_slider->setFixedHeight(4);
    d->m_slider->setCursor(Qt::sizeVerCursor);
    break;
  }

  if (box)
    box->activate();

  d->m_sliding = false;

  d->m_count = 0;
}


KTabZoomFrame::~KTabZoomFrame()
{
  delete d;
}


void KTabZoomFrame::slotCloseButtonClicked()
{
  emit closeClicked();
}


void KTabZoomFrame::setDockMode(bool docked)
{
  d->m_closeButton->setEnabled(!docked);
  d->m_dockButton->setOn(docked);
}


void KTabZoomFrame::slotDockButtonToggled(bool toggle)
{
  setDockMode(toggle);

  emit dockToggled(toggle);
}


int KTabZoomFrame::addTab(QWidget *widget, const QString &title)
{
  int index = d->m_count++;

  KTZFWidgetInfo *info = new KTZFWidgetInfo;

  info->m_index = d->m_stack->addWidget(widget, index);
  info->m_widget = widget;
  info->m_title = title;

  d->m_info.append(info);

  return index;
}


void KTabZoomFrame::removeTab(int index)
{
  for (KTZFWidgetInfo *i=d->m_info.first(); i != 0; i = d->m_info.next())
    if (i->m_index == index)
    {
      d->m_info.remove(i);
      return;
    }
}


void KTabZoomFrame::selected(int index)
{
  for (KTZFWidgetInfo *i=d->m_info.first(); i != 0; i = d->m_info.next())
    if (i->m_index == index)
    {
      d->m_stack->raiseWidget(i->m_widget);
      i->m_widget->setFocus();
      d->m_title->setText(i->m_title);
      i->m_widget->setFocus();
      return;
    }
}



void KTabZoomFrame::mousePressEvent(QMouseEvent *ev)
{
  if (d->m_sliding || ev->button() != LeftButton)
    return;

  if (!d->m_slider->geometry().contains(ev->pos()))
    return;

  d->m_sliding = true;
  d->m_slideStart = ev->globalPos();

  switch (d->m_position)
  {
  case KTabZoomPosition::Left:
    d->m_initialSize = width();
    break;

  case KTabZoomPosition::Right:
    d->m_initialSize = width();
    d->m_initialPos = x() + width();
    break;

  case KTabZoomPosition::Top:
    d->m_initialSize = height();
    break;

  case KTabZoomPosition::Bottom:
    d->m_initialSize = height();
    d->m_initialPos = y()+height();
    break;
  }
}


void KTabZoomFrame::mouseReleaseEvent(QMouseEvent *)
{
  if (d->m_sliding)
    d->m_sliding = false;

  emit sizeChanged();
}


void KTabZoomFrame::mouseMoveEvent(QMouseEvent *ev)
{
  if (!d->m_sliding)
    return;

  int extend;
  switch (d->m_position)
  {
  case KTabZoomPosition::Left:
    extend = ev->globalPos().x() - d->m_slideStart.x() + d->m_initialSize;
    if (extend < 250)
      extend = 250;
    resize(extend, height());
    break;

  case KTabZoomPosition::Right:
    extend = d->m_slideStart.x() - ev->globalPos().x() + d->m_initialSize;
    if (extend < 250)
      extend = 250;
    setGeometry(d->m_initialPos - extend, y(), extend, height());
    break;

  case KTabZoomPosition::Top:
    extend = ev->globalPos().y() - d->m_slideStart.y() + d->m_initialSize;
    if (extend < 125)
      extend = 125;
    resize(width(), extend);
    break;

  case KTabZoomPosition::Bottom:
    extend = d->m_slideStart.y() - ev->globalPos().y() + d->m_initialSize;
    if (extend < 125)
      extend = 125;
    setGeometry(x(), d->m_initialPos - extend, width(), extend);
    qDebug("geom %d %d %d %d", x(), d->m_initialPos - extend, width(),
	extend);
    break;
  }

  emit sizeChanged();
}


#include "ktabzoomframe.moc"
