#include <qintdict.h>
#include <qpainter.h>
#include <qlayout.h>
#include <qframe.h>
#include <qsignalmapper.h>


#include <kdebug.h>


#include "ktabzoombutton.h"


#include "ktabzoombar.h"


class KTabZoomBarPrivate
{
public:

  KTabZoomPosition::Position m_tabPosition;
  int                        m_selected;
  QBoxLayout                 *m_layout;
  QSignalMapper              *m_mapper;
  QIntDict<KTabZoomButton>   m_buttons;
  int                        m_count;

};



KTabZoomBar::KTabZoomBar(QWidget *parent, KTabZoomPosition::Position pos, const char *name)
  : QWidget(parent, name)
{
  d = new KTabZoomBarPrivate;

  d->m_tabPosition = pos;

  if (pos == KTabZoomPosition::Left || pos == KTabZoomPosition::Right)
  {
    setFixedWidth(fontMetrics().height() + 2);
    d->m_layout = new QVBoxLayout(this);
    d->m_layout = new QVBoxLayout(d->m_layout);
    d->m_layout->setDirection(QBoxLayout::BottomToTop);
  }
  else
  {
    setFixedHeight(fontMetrics().height() + 2);
    d->m_layout = new QHBoxLayout(this);
    d->m_layout = new QHBoxLayout(d->m_layout);
    d->m_layout->setDirection(QBoxLayout::RightToLeft);
  }
  d->m_layout->addStretch();

  d->m_selected = -1;

  d->m_mapper = new QSignalMapper(this);
  connect(d->m_mapper, SIGNAL(mapped(int)), this, SLOT(toggled(int)));
}


KTabZoomBar::~KTabZoomBar()
{
  delete d;
}


int KTabZoomBar::addTab(QTab *tab)
{
  KTabZoomButton *btn = new KTabZoomButton(tab->text(), this, d->m_tabPosition);
  d->m_layout->addWidget(btn);
  btn->show();

  int index = d->m_count++;

  d->m_buttons.insert(index, btn);

  d->m_mapper->setMapping(btn, index);
  connect(btn, SIGNAL(toggled(bool)), d->m_mapper, SLOT(map()));

  return index;
}


void KTabZoomBar::removeTab(int index)
{
  kdDebug(9000) << "KTabZoomBar::removeTab" << index << endl;

  KTabZoomButton *button = d->m_buttons[index];
  if (!button)
    return;

  delete button;
  d->m_buttons.remove(index);
}


void KTabZoomBar::toggled(int index)
{
  static bool guard=false;
  if (guard)
    return;

  KTabZoomButton *button = d->m_buttons[index];
  if (!button)
    return;

  bool on = button->isOn();

  if (on)
  {
    guard = true;
    QIntDictIterator<KTabZoomButton> it(d->m_buttons);
    while (it.current())
    {
      if (it.currentKey() != index)
        it.current()->setOn(false);
      ++it;
    }
    guard = false;

    emit selected(index);
  }
  else
    emit unselected();
}


void KTabZoomBar::setPressed(int index, bool pressed)
{
  KTabZoomButton *button = d->m_buttons[index];
  if (!button)
    return;
    
  button->setOn(pressed);
}


void KTabZoomBar::unsetButtons()
{
  QIntDictIterator<KTabZoomButton> it(d->m_buttons);
  while (it.current())
  {
    it.current()->setOn(false);
    ++it;
  }
}
		

#include "ktabzoombar.moc"
