#include <qpopupmenu.h>

#include <klocale.h>
#include <kapplication.h>
#include <kiconloader.h>
#include <kdebug.h>
#include "ktabwidget.h"

KTabWidget::KTabWidget(QWidget *parent, const char *name) : QTabWidget(parent,name)
{
  m_pTabBar = new KTabBar(this, "tabbar");
  setTabBar(m_pTabBar);
  connect(m_pTabBar, SIGNAL(closeWindow(const QWidget*)), this, SIGNAL(closeWindow(const QWidget*)));
  connect(m_pTabBar, SIGNAL(closeOthers(const QWidget*)), this, SIGNAL(closeOthers(const QWidget*)));
}

KTabWidget::~KTabWidget()
{
}

KTabBar::KTabBar(QWidget *parent, const char *name) : QTabBar(parent,name)
{
  m_pPopupMenu = new QPopupMenu(this);

  QPixmap closePixmap = KGlobal::instance()->iconLoader()->loadIcon( "tab_remove", KIcon::Small, 0, KIcon::DefaultState, 0, true ); 
  if (closePixmap.isNull())
    closePixmap = SmallIcon("fileclose");

  m_pPopupMenu->insertItem(closePixmap, i18n("&Close"), this, SLOT(closeWindowSlot()));
  m_pPopupMenu->insertItem(i18n("Close &Others"), this, SLOT(closeOthersSlot()));
}

void KTabBar::closeWindowSlot()
{
	emit closeWindow(m_pPage);
}

void KTabBar::closeOthersSlot()
{
	emit closeOthers(m_pPage);
}

void KTabBar::mousePressEvent(QMouseEvent *e)
{
  if(e->button() == RightButton) {
    QTab *tab = selectTab(e->pos() );
	if( tab== 0L ) return;
	m_pPage = ((KTabWidget*)parent())->page(indexOf(tab->identifier() ) );
	if(m_pPage == 0L) return;
    //(KTabWidget*)parent())->showPage(m_pPage);    
	m_pPopupMenu->exec(mapToGlobal(e->pos()));
  }
  QTabBar::mousePressEvent(e);
}

KTabBar::~KTabBar()
{
}

#include "ktabwidget.moc"
