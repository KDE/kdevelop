#include <qintdict.h>
#include <qpainter.h>
#include <qlayout.h>
#include <qframe.h>
#include <qsignalmapper.h>


#include <kdebug.h>


#include "ktabzoombutton.h"
#include "ktabzoombarlayout.h"


#include "ktabzoombar.h"


class KTabZoomBarPrivate
{
public:

  KTabZoomPosition::Position m_tabPosition;
  int                        m_selected;
  KTabZoomBarLayout          *m_layout;
  QSignalMapper              *m_mapper;
  QIntDict<KTabZoomButton>   m_buttons;
  int                        m_count;

};


KTabZoomBar::KTabZoomBar(QWidget *parent, KTabZoomPosition::Position pos, const char *name)
  : QWidget(parent, name)
{
  d = new KTabZoomBarPrivate;

  d->m_tabPosition = pos;

  d->m_layout = new KTabZoomBarLayout(this, pos);

  if (pos == KTabZoomPosition::Left || pos == KTabZoomPosition::Right)
    setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred));
  else
    setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum));

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
  d->m_layout->add(btn);
  btn->show();

  int index = d->m_count++;

  d->m_buttons.insert(index, btn);

  d->m_mapper->setMapping(btn, index);
  connect(btn, SIGNAL(toggled(bool)), d->m_mapper, SLOT(map()));

  return index;
}


void KTabZoomBar::removeTab(int index)
{
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


QSize KTabZoomBar::sizeHint() const
{
  return d->m_layout->sizeHint();
}


int KTabZoomBar::heightForWidth(int width) const
{
  return d->m_layout->heightForWidth(width);
}

#include "ktabzoombar.moc"
