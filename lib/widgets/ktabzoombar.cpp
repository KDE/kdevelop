#include <qintdict.h>
#include <qpainter.h>
#include <qlayout.h>
#include <qframe.h>
#include <qsignalmapper.h>
#include <qtooltip.h>


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
  QSignalMapper              *m_clickedMapper;
  QIntDict<KTabZoomButton>   m_buttons;
  int                        m_count;
  bool                       m_docked;
  
};


KTabZoomBar::KTabZoomBar(QWidget *parent, KTabZoomPosition::Position pos, const char *name)
  : QWidget(parent, name)
{
  d = new KTabZoomBarPrivate;

  d->m_tabPosition = pos;
  d->m_docked = false;
  d->m_count = 0;

  d->m_layout = new KTabZoomBarLayout(this, pos);

  if (pos == KTabZoomPosition::Left || pos == KTabZoomPosition::Right)
    setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred));
  else
    setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum));

  d->m_selected = -1;

  d->m_clickedMapper = new QSignalMapper(this);
  connect(d->m_clickedMapper, SIGNAL(mapped(int)), this, SLOT(clicked(int)));
}


KTabZoomBar::~KTabZoomBar()
{
  delete d;
}


int KTabZoomBar::addTab(const QTab &tab, const QString& toolTip)
{
  KTabZoomButton *btn = new KTabZoomButton(tab.text(), this, d->m_tabPosition);
  QToolTip::add( btn, toolTip );
  d->m_layout->add(btn);
  btn->show();

  int index = d->m_count++;

  d->m_buttons.insert(index, btn);

  d->m_clickedMapper->setMapping(btn, index);
  connect(btn, SIGNAL(clicked()), d->m_clickedMapper, SLOT(map()));

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


void KTabZoomBar::clicked(int index)
{
  KTabZoomButton *button = d->m_buttons[index];
  if (!button)
    return;

  if (button->isOn())
  {
    setActiveIndex(index);
  }
  else
  {
    emit unselected();
  }
}


void KTabZoomBar::setActiveIndex(int index)
{
  KTabZoomButton *button = d->m_buttons[index];
  if (!button)
    return;
  QIntDictIterator<KTabZoomButton> it(d->m_buttons);
  while (it.current())
  {
    if (it.currentKey() != index)
      it.current()->setOn(false);
    ++it;
  }
  button->setOn(true);
  emit selected(index);
}


void KTabZoomBar::unsetButtons()
{
  QIntDictIterator<KTabZoomButton> it(d->m_buttons);
  while (it.current())
  {
    if (it.current()->isOn()) {
      it.current()->setOn(false);
      emit unselected();
      return;
    }
    ++it;
  }
}


void KTabZoomBar::setDockMode(bool docked)
{
  d->m_docked = docked;
}


#include "ktabzoombar.moc"
