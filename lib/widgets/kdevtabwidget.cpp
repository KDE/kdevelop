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

#include <q3popupmenu.h>
//Added by qt3to4:
#include <QPixmap>
#include <QMouseEvent>

#include <kglobal.h>
#include <klocale.h>
#include <kicontheme.h>
#include <kiconloader.h>

#include "kdevtabwidget.h"

KDevTabWidget::KDevTabWidget(QWidget *parent, const char *name) : QTabWidget(parent,name)
{
  m_pTabBar = new KTabBar(this, "tabbar");
  setTabBar(m_pTabBar);
  connect(m_pTabBar, SIGNAL(closeWindow(const QWidget*)), this, SIGNAL(closeWindow(const QWidget*)));
  connect(m_pTabBar, SIGNAL(closeOthers(QWidget*)), this, SIGNAL(closeOthers(QWidget*)));
}

KTabBar::KTabBar(QWidget *parent, const char *name) : QTabBar(parent,name)
{
  m_pPopupMenu = new Q3PopupMenu(this);

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
  if(e->button() == Qt::RightButton) {

    QTab *tab = selectTab(e->pos() );
    if( tab == 0L ) return;
    
    m_pPage = ((KDevTabWidget*)parent())->page(indexOf(tab->identifier() ) );
    if(m_pPage == 0L) return;

    m_pPopupMenu->exec(mapToGlobal(e->pos()));
  }
  QTabBar::mousePressEvent(e);
}

#include "kdevtabwidget.moc"
