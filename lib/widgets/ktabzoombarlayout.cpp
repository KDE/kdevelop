#include <kdebug.h>


#include "ktabzoombarlayout.h"


class KTabZoomBarLayoutIterator : public QGLayoutIterator
{
public:

  KTabZoomBarLayoutIterator(QPtrList<QLayoutItem> *l)
    : idx(0), list(l) 
  { 
  }

  QLayoutItem *current()
  { 
    return idx < int(list->count()) ? list->at(idx) : 0;  
  }

  QLayoutItem *next()
  { 
    idx++; 
    return current(); 
  }

  QLayoutItem *takeCurrent()
  { 
    return list->take(idx); 
  }

private:

  int idx;
  QPtrList<QLayoutItem> *list;

};




KTabZoomBarLayout::KTabZoomBarLayout(QWidget *parent, KTabZoomPosition::Position pos)
  : QLayout(parent, 0, 2), m_pos(pos)
{
  m_fontHeight = parent->fontMetrics().height();
}

KTabZoomBarLayout::KTabZoomBarLayout(QLayout *parent, KTabZoomPosition::Position pos)
  : QLayout(parent, 2), m_pos(pos), m_fontHeight(10)
{
}


KTabZoomBarLayout::KTabZoomBarLayout(KTabZoomPosition::Position pos)
  : QLayout(2), m_pos(pos), m_fontHeight(10)
{
}


KTabZoomBarLayout::~KTabZoomBarLayout()
{
  deleteAllItems();
}


void KTabZoomBarLayout::addItem(QLayoutItem *item)
{
  m_list.append(item);
}


QSize KTabZoomBarLayout::sizeHint() const
{
  QSize sh = QSize(100,100);
  if ((m_pos == KTabZoomPosition::Top) || (m_pos == KTabZoomPosition::Bottom))
    sh = QSize(completeSize(), m_fontHeight + 2);
  else
    sh = QSize(m_fontHeight + 2, completeSize());

  return sh;
}


QSize KTabZoomBarLayout::minimumSize() const
{
  if ((m_pos == KTabZoomPosition::Top) || (m_pos == KTabZoomPosition::Bottom))
    return QSize(completeSize(), m_fontHeight + 2);
  else
    return QSize(m_fontHeight + 2, completeSize());
}


QLayoutIterator KTabZoomBarLayout::iterator()
{
  return QLayoutIterator(new KTabZoomBarLayoutIterator(&m_list));
}


void KTabZoomBarLayout::setGeometry(const QRect &rect)
{
  QLayout::setGeometry(rect);

  QPtrListIterator<QLayoutItem> it(m_list);
  if (it.count() == 0)
    return;

  QLayoutItem *o;
  int i = 0;

  if ((m_pos == KTabZoomPosition::Top) || (m_pos == KTabZoomPosition::Bottom))
  {
    int w=0, h=0;

    while ((o = it.current()) != 0) 
    {
      ++it;

      QSize sh = o->sizeHint();
     
      if (w + sh.width() > rect.width())
      {
	w = 0;
	h = h + m_fontHeight + 2;
      }
      
      o->setGeometry(QRect(w, h, sh.width(), sh.height()));
      w = w + sh.width() + spacing();
    
      ++i;
    }
  }
  else
  {
    int h=0;

    while ((o = it.current()) != 0)
    {
      ++it;
			            
      QSize sh = o->sizeHint();

      o->setGeometry(QRect(0, h, sh.width(), sh.height()));
      h = h + sh.height() + spacing();
      
      ++i;
    }
  }
}


bool KTabZoomBarLayout::hasHeightForWidth () const
{
  return (m_pos == KTabZoomPosition::Top) || (m_pos == KTabZoomPosition::Bottom);
}


int KTabZoomBarLayout::heightForWidth (int width) const
{
  if ((m_pos == KTabZoomPosition::Left) || (m_pos == KTabZoomPosition::Right))
    return -1;

  int height = m_fontHeight + 2;

  int w = 0;
  
  QPtrListIterator<QLayoutItem> it(m_list);
  QLayoutItem *o;
  while ((o = it.current()) != 0)
  {
    ++it;
    
    QSize sh = o->sizeHint();

    if ((w + sh.width()) < width)
      w = w + sh.width() + spacing();
    else
    {
      height = height + m_fontHeight + 2;
      w = sh.width() + spacing();
    }
  }

  return height;
}


int KTabZoomBarLayout::completeSize() const
{
  QPtrListIterator<QLayoutItem> it(m_list);

  int s = spacing() * (it.count() - 1);

  QLayoutItem *o;
  while ((o = it.current()) != 0)
  {
    ++it;

    QSize sh = o->sizeHint();

    if ((m_pos == KTabZoomPosition::Left) || (m_pos == KTabZoomPosition::Right))
      s += sh.width();
    else
      s += sh.height();
  }

  return s;
}
