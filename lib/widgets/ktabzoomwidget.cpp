// TODO: Clean up the different usages of the different indices!


#include <qptrlist.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qwidgetstack.h>
#include <qguardedptr.h>

#include <kdebug.h>
#include <kconfig.h>


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
  bool 			     m_docked;
  QWidget		     *m_strut;
  QGuardedPtr<QWidget>       m_lastActiveWidget;
};


KTabZoomWidget::KTabZoomWidget(QWidget *parent, KTabZoomPosition::Position pos, const char *name)
  : QWidget(parent, name)
{
  d = new KTabZoomWidgetPrivate;
  d->m_info.setAutoDelete(true);

  d->m_tabPosition = pos;
  d->m_content = 0;
  d->m_docked = false;
  d->m_strut = 0;
  d->m_lastActiveWidget = 0;

  d->m_tabBar = new KTabZoomBar(this, pos);
  connect(d->m_tabBar, SIGNAL(selected(int)), this, SLOT(selected(int)));
  connect(d->m_tabBar, SIGNAL(unselected()), this, SLOT(unselected()));

  if (pos == KTabZoomPosition::Left || pos == KTabZoomPosition::Right)
    d->m_layout = new QHBoxLayout(this);
  else
    d->m_layout = new QVBoxLayout(this);

  d->m_popup = new KTabZoomFrame(parent, pos);

  connect(d->m_popup, SIGNAL(closeClicked()), this, SLOT(unselected()));
  connect(d->m_popup, SIGNAL(dockToggled(bool)), this, SLOT(setDockMode(bool)));
  connect(d->m_popup, SIGNAL(sizeChanged()), this, SLOT(adjustStrut()));

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
  KTZWidgetInfo *info = new KTZWidgetInfo;
  info->m_widget = widget;

  info->m_barIndex = d->m_tabBar->addTab( QTab(title) );
  info->m_index = d->m_popup->addTab(widget, title);

  connect(widget, SIGNAL(destroyed()), this, SLOT(widgetDeleted()));

  d->m_info.append(info);
}


void KTabZoomWidget::widgetDeleted()
{
  const QWidget *w = static_cast<const QWidget*>(sender());

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

  d->m_strut = new QWidget(this);

  switch (d->m_tabPosition)
  {
  case KTabZoomPosition::Left:
  case KTabZoomPosition::Top:
    d->m_layout->addWidget(d->m_tabBar);
    d->m_layout->addWidget(d->m_strut);
    d->m_layout->addWidget(d->m_content,1);
    break;

  case KTabZoomPosition::Right:
  case KTabZoomPosition::Bottom:
    d->m_layout->addWidget(d->m_content,1);
    d->m_layout->addWidget(d->m_strut);
    d->m_layout->addWidget(d->m_tabBar);
    break;
  }

  d->m_strut->hide();

  content->show();
}


void KTabZoomWidget::selected(int index)
{
  qDebug( "KTabZoomWidget::selected" );

  calculateGeometry();

  if (d->m_docked)
  {
    d->m_strut->show();
    adjustStrut();
  }
  
  for (KTZWidgetInfo *i=d->m_info.first(); i != 0; i = d->m_info.next())
    if (i->m_barIndex == index)
    {
      d->m_popup->selected(i->m_index);
      d->m_popup->show();
      d->m_lastActiveWidget = i->m_widget;
      return;
    }
}


void KTabZoomWidget::unselected()
{
  d->m_popup->hide();
  d->m_tabBar->unsetButtons();
  d->m_strut->hide();
}


void KTabZoomWidget::raiseWidget(QWidget *widget)
{
  qDebug( "KTabZoomWidget::raiseWidget" );
  if ( !widget )
    widget = d->m_lastActiveWidget;
  for (KTZWidgetInfo *i=d->m_info.first(); i != 0; i = d->m_info.next())
    if (i->m_widget == widget || !widget)
    {
      d->m_tabBar->setActiveIndex(i->m_barIndex);
      d->m_lastActiveWidget = i->m_widget;
      return;
    }
}


void KTabZoomWidget::lowerAllWidgets()
{
  d->m_tabBar->unsetButtons();
}


void KTabZoomWidget::lowerWidget(QWidget *w)
{
  if (d->m_docked)
    return;

  for (KTZWidgetInfo *i=d->m_info.first(); i != 0; i = d->m_info.next())
    if (i->m_widget == w)
    {
      d->m_popup->hide();
      d->m_tabBar->unsetButtons();
      return;
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


void KTabZoomWidget::resizeEvent(QResizeEvent *ev)
{
  QWidget::resizeEvent(ev);

  calculateGeometry();
}


void KTabZoomWidget::setDockMode(bool docked)
{
  d->m_docked = docked;

  d->m_tabBar->setDockMode(docked);
  d->m_popup->setDockMode(docked);

  if (!docked)
  {
    d->m_strut->hide();
    return;
  }

  d->m_strut->show();

  adjustStrut();
}


void KTabZoomWidget::saveSettings(KConfig *config)
{
  config->writeEntry("Docked", d->m_docked);
  if (d->m_tabPosition == KTabZoomPosition::Left || d->m_tabPosition == KTabZoomPosition::Right)
    config->writeEntry("Strut", d->m_popup->width());
  else
    config->writeEntry("Strut", d->m_popup->height());
}


void KTabZoomWidget::loadSettings(KConfig *config)
{
  int s = config->readNumEntry("Strut", -1);
  if (s > 0)
  {
    if (d->m_tabPosition == KTabZoomPosition::Left || d->m_tabPosition == KTabZoomPosition::Right)
      d->m_popup->resize(s, d->m_popup->height());
    else
      d->m_popup->resize(d->m_popup->width(), s);
  }

  setDockMode(config->readBoolEntry("Docked", false));

  if (d->m_docked)
  {
    KTZWidgetInfo *i=d->m_info.first();
    if (i) {
      d->m_tabBar->setActiveIndex(i->m_barIndex);
    } else {
      // np parts there to show so we just hide ourselves
      setDockMode( false );
    }
  }
}


void KTabZoomWidget::adjustStrut()
{
  if (!d->m_docked)
    return;

  if (d->m_tabPosition == KTabZoomPosition::Left || d->m_tabPosition == KTabZoomPosition::Right)
    d->m_strut->setFixedWidth(d->m_popup->width());
  else
    d->m_strut->setFixedHeight(d->m_popup->height());
}


#include "ktabzoomwidget.moc"
