// TODO: Clean up the different usages of the different indices!


#include <qptrlist.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qwidgetstack.h>


#include <kdebug.h>


#include "ktabzoomframe.h"


#include "ktabzoomwidget.h"


class KTZWidgetInfo
{
public:

  QWidget *m_widget;
  int     m_index, m_barIndex;

};


class KTabZoomWidgetPrivate
{
public:

  KTabZoomPosition::Position m_tabPosition;
  QWidget                    *m_content;
  KTabZoomBar                *m_tabBar;
  QBoxLayout                 *m_layout;
  KTabZoomFrame              *m_popup;
  QPtrList<KTZWidgetInfo>    m_info;

};


KTabZoomWidget::KTabZoomWidget(QWidget *parent, KTabZoomPosition::Position pos, const char *name)
  : QWidget(parent, name)
{
  d = new KTabZoomWidgetPrivate;
  d->m_info.setAutoDelete(true);

  d->m_tabPosition = pos;
  d->m_content = 0;

  d->m_tabBar = new KTabZoomBar(this, pos);
  connect(d->m_tabBar, SIGNAL(selected(int)), this, SLOT(selected(int)));
  connect(d->m_tabBar, SIGNAL(unselected()), this, SLOT(unselected()));

  if (pos == KTabZoomPosition::Left || pos == KTabZoomPosition::Right)
    d->m_layout = new QHBoxLayout(this);
  else
    d->m_layout = new QVBoxLayout(this);

  d->m_popup = new KTabZoomFrame(parent, pos);

  connect(d->m_popup, SIGNAL(closeClicked()), this, SLOT(unselected()));

  d->m_popup->hide();

  if (pos == KTabZoomPosition::Left || pos == KTabZoomPosition::Right)
    d->m_popup->resize(250, height());
  else
    d->m_popup->resize(width(), 125);
}


KTabZoomWidget::~KTabZoomWidget()
{
  delete d;
}


void KTabZoomWidget::addTab(QWidget *widget, const QString &title)
{
  kdDebug(9000) << "AddTab: " << widget << endl;

  KTZWidgetInfo *info = new KTZWidgetInfo;
  info->m_widget = widget;

  info->m_barIndex = d->m_tabBar->addTab(new QTab(title));
  info->m_index = d->m_popup->addTab(widget, title);

  connect(widget, SIGNAL(destroyed()), this, SLOT(widgetDeleted()));

  d->m_info.append(info);
}


void KTabZoomWidget::widgetDeleted()
{
  const QWidget *w = static_cast<const QWidget*>(sender());

  kdDebug(9000) << "Widget deleted: " << w << endl;

  for (KTZWidgetInfo *i=d->m_info.first(); i != 0; i = d->m_info.next())
    if (i->m_widget == w)
    {
      d->m_tabBar->removeTab(i->m_barIndex);
      d->m_popup->removeTab(i->m_index);
      d->m_info.remove(i);
      return;
    }
}


void KTabZoomWidget::addContent(QWidget *content)
{
  // only accept one child
  if (d->m_content != 0)
    return;

  d->m_content = content;

  switch (d->m_tabPosition)
  {
  case KTabZoomPosition::Left:
  case KTabZoomPosition::Top:
    d->m_layout->addWidget(d->m_tabBar);
    d->m_layout->addWidget(d->m_content);
    break;

  case KTabZoomPosition::Right:
  case KTabZoomPosition::Bottom:
    d->m_layout->addWidget(d->m_content);
    d->m_layout->addWidget(d->m_tabBar);
    break;
  }

  content->show();
}


void KTabZoomWidget::selected(int index)
{
  calculateGeometry();

  for (KTZWidgetInfo *i=d->m_info.first(); i != 0; i = d->m_info.next())
    if (i->m_barIndex == index)
    {
      d->m_popup->selected(i->m_index);
      d->m_popup->show();
    }
}


void KTabZoomWidget::raiseWidget(QWidget *widget)
{
  for (KTZWidgetInfo *i=d->m_info.first(); i != 0; i = d->m_info.next())
    if (i->m_widget == widget)
    {
      d->m_tabBar->setPressed(i->m_barIndex);
      return;
    }
}


void KTabZoomWidget::lowerAllWidgets()
{
    for ( KTZWidgetInfo* i = d->m_info.first(); i != 0; i = d->m_info.next() )
    {
        d->m_popup->hide();
        d->m_tabBar->unsetButtons();
    }
}


void KTabZoomWidget::calculateGeometry()
{
  switch (d->m_tabPosition)
  {
  case KTabZoomPosition::Left:
    d->m_popup->setGeometry(d->m_tabBar->width(), y(), d->m_popup->width(), height());
    break;

  case KTabZoomPosition::Right:
    d->m_popup->setGeometry(d->m_tabBar->x() - d->m_popup->width(), y(), d->m_popup->width(), height());
    break;

  case KTabZoomPosition::Top:
    d->m_popup->setGeometry(x(), d->m_tabBar->height(), width(), d->m_popup->height());
    break;

  case KTabZoomPosition::Bottom:
    d->m_popup->setGeometry(x(), d->m_tabBar->y() - d->m_popup->height(), width(), d->m_popup->height());
    break;
  }
}


void KTabZoomWidget::unselected()
{
  kdDebug(9000) << "Unselected" << endl;

  d->m_popup->hide();
  d->m_tabBar->unsetButtons();
}


void KTabZoomWidget::lowerWidget(QWidget *w)
{
  for (KTZWidgetInfo *i=d->m_info.first(); i != 0; i = d->m_info.next())
    if (i->m_widget == w)
    {
      d->m_popup->hide();
      d->m_tabBar->unsetButtons();
      return;
    }
}


void KTabZoomWidget::resizeEvent(QResizeEvent *ev)
{
  QWidget::resizeEvent(ev);

  calculateGeometry();
}


#include "ktabzoomwidget.moc"
