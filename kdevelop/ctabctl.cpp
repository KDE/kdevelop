/***************************************************************************
                     ctabctl.cpp - a ktabctl class with a setCurrentTab   
                             -------------------                                         

    version              :                                   
    begin                : 22 Jul 1998                                        
    copyright            : (C) 1998 by Sandy Meier                         
    email                : smeier@rz.uni-potsdam.de                                     
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/

#include "ctabctl.h"

#include <qobjectlist.h>
#include <qtabbar.h>

// **************************************************************************

CTabCtl::CTabCtl( QWidget* parent, const char* name, const QString& type) :
  QTabWidget(parent,name)
{
  setFocusPolicy(QWidget::NoFocus);
//  tabs->setFocusPolicy(QWidget::NoFocus);
  setTabPosition(Top);
  if(type == "normal"){
    setTabShape(Rounded);
  }
  if(type == "output_widget"){
    setTabShape(Rounded);
    //    tabs->setFont(QFont("helvetica",10));
  }

  connect(this, SIGNAL(currentChanged(QWidget *)), SLOT(slotCurrentChanged(QWidget *)));
}

// **************************************************************************

void CTabCtl::setCurrentTab(int id)
{
  tabBar()->setCurrentTab(id);
}

// **************************************************************************

int CTabCtl::getCurrentTab()
{
  return tabBar()->currentTab();
}

// **************************************************************************

void CTabCtl::setTabEnabled(const char* name, bool enabled)
{
  if ( !name )
    return;

  QObjectList * l
      = this->queryList( "QWidget", name, FALSE, TRUE );

  // There should only be one here
  ASSERT(l && (l->count() == 1));
  QObject *o = l->first();
  ASSERT( o->isWidgetType() );
  QTabWidget::setTabEnabled( (QWidget*)o, enabled );
  delete l;
}

// **************************************************************************

void CTabCtl::slotCurrentChanged(QWidget *)
{
  emit tabSelected(currentPageIndex());
}

// **************************************************************************

// **************************************************************************
// **************************************************************************
// **************************************************************************

//CTabCtl::CTabCtl( QWidget* parent, const char* name, const QString&) :
//  KDockTabCtl(parent,name)
//{
//  setFocusPolicy(QWidget::NoFocus);
//  tabs->setFocusPolicy(QWidget::NoFocus);
////  if(type == "normal"){
////    setShape(QTabBar::RoundedAbove);
////  }
////  if(type == "output_widget"){
////    setShape(QTabBar::RoundedAbove);
////    //    tabs->setFont(QFont("helvetica",10));
////  }
//
//  connect(this, SIGNAL(pageSelected(QWidget *)), SLOT(slotPageSelected(QWidget *)));
//}
//
//// **************************************************************************
//
//void CTabCtl::setCurrentTab(int id)
//{
//  tabs->setCurrentTab(id);
////  tabSelected(id);
////  showTab(id);
//}
//
//// **************************************************************************
//
//int CTabCtl::getCurrentTab()
//{
//  int currentTab=tabs->currentTab();
//  return currentTab;
//}
//
//// **************************************************************************
//
//void CTabCtl::addTab(QWidget* widget, const QString& label)
//{
//  insertPage(widget, label);
//  setPageCaption (widget, label);
//}
//
//// **************************************************************************
//
//void CTabCtl::addTab(QWidget* widget, const QPixmap& pixmap, const QString& label)
//{
//  int id = insertPage(widget, label);
//  tabs->setPixmap(id, pixmap);
//  showTabIcon(true);
//  setPageCaption (widget, label);
//}
//
//// **************************************************************************
//
//void CTabCtl::changeTab(QWidget* widget, const QPixmap& pixmap, const QString& label)
//{
//  int i = id(widget);
//  tabs->setPixmap(i, pixmap);
//  showTabIcon(true);
//  setPageCaption (widget, label);
//}
//
//// **************************************************************************
//
//void CTabCtl::setTabEnabled(const char* name, bool enabled)
//{
//  if ( !name )
//    return;
//
//  QObjectList * l
//      = this->queryList( "QWidget", name, false, true);
//
//  // There should only be one here
//  ASSERT(l && (l->count() == 1));
//  QObject *o = l->first();
//  ASSERT( o->isWidgetType() );
//  setPageEnabled( (QWidget*)o, enabled );
//  delete l;
//}
//
//// **************************************************************************
//
//void CTabCtl::slotPageSelected(QWidget *w)
//{
//  emit tabSelected(index(w));
//}
//
//// **************************************************************************
#include "ctabctl.moc"
