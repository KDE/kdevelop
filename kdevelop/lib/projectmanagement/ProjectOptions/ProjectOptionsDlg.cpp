/***************************************************************************
                          ProjectOptionsDlg.cpp  -  description
                             -------------------
    begin                : Sat Jan 27 2001
    copyright            : (C) 2001 by Omid Givi
    email                : omid@givi.nl
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "ProjectOptionsDlg.h"
#include "ProjectOptionsDlgBase.h"
#include <kdebug.h>
#include <qheader.h>
#include <qlistview.h>
#include <kiconloader.h>
#include "GeneralPage.h"
#include "DefaultPage.h"
#include "NotImplementedPage.h"
#include <qlayout.h>
#include <qobjectlist.h>
#include "kdialogbase.h"
#include "klocale.h"
#include <qhbox.h>
#include <qvbox.h>

/*
 * class ProjectOptionsDlg
 */

ProjectOptionsDlg::ProjectOptionsDlg(QWidget *parent, const char *name, ProjectSpace *ps)
    : ProjectOptionsDlgBase( parent, name, true)
{
  m_ps = ps;

  // Where do I get the right debug number for 'this'??
  kdDebug(9000) << "Opening ProjectOptions Dialog" << endl;

  categoryTreeList->header()->hide();
	categoryTreeList->setRootIsDecorated(true);
  categoryTreeList->setSorting(-1);

	// general page
	QListViewItem* generalItem = new QListViewItem( categoryTreeList, "General");
	generalItem->setPixmap(0, KGlobal::iconLoader()->loadIcon("folder", KIcon::Desktop, KIcon::SizeSmall));
	QListViewItem* compilersItem = new QListViewItem( categoryTreeList, "Compilers");
	compilersItem->setPixmap(0, KGlobal::iconLoader()->loadIcon("folder", KIcon::Desktop, KIcon::SizeSmall));
	compilersItem->setOpen(true);
		
	dp = new DefaultPage(localWidget, "Default Page");
  nop= new NotImplementedPage(localWidget, "Not Implemented Page");
//  nop->resize(localWidget->width(), localWidget->height());
	treeListToWidget.insert(generalItem, dp);
	treeListToWidget.insert(compilersItem, dp);
  generalItem = new QListViewItem(generalItem, "Info");
	generalItem->setPixmap(0, KGlobal::iconLoader()->loadIcon("info", KIcon::Desktop, KIcon::SizeSmall));

	// iterate the compilers
	QListViewItem* compilerItem;
	QList<KDevCompiler> *compilers = ps->compilers();
  QListIterator<KDevCompiler> it(*compilers);
  for ( ; it.current(); ++it ) {
    KDevCompiler *comp = it.current();
    compilerItem = new QListViewItem(compilersItem, *(comp->name()));
    compilerItem->setPixmap(0, KGlobal::iconLoader()->loadIcon(*(comp->icon()), KIcon::Desktop, KIcon::SizeSmall));
    QWidget* optionsWidget = comp->initOptionsWidget(localWidget, this);
    optionsWidget->resize(localWidget->width(), localWidget->height());
    treeListToWidget.insert(compilerItem, comp->optionsWidget());
	  emit(WidgetStarted(comp));
  }

  connect(categoryTreeList,SIGNAL(clicked(QListViewItem*)), SLOT(slotTreeListItemSelected(QListViewItem*)));
//  connect(buttonOk, SIGNAL(clicked()), this, SLOT(slotButtonOkClicked()));	 		

  dp->raise();
	currentWidget = dp;
}

ProjectOptionsDlg::~ProjectOptionsDlg(){
}

void ProjectOptionsDlg::slotTreeListItemSelected(QListViewItem* item){

  if (!item) return; // do we have an item?
  if(item->depth() != 1) return; // do we have the right item?

  QWidget *widgetItem = treeListToWidget[item];

  QString name = item->text(0);
  if (widgetItem){
	  widgetItem->raise();
	  currentWidget = widgetItem;
	  if (widgetItem->parentWidget()){
  	  widgetItem->resize((widgetItem->parentWidget())->width(), (widgetItem->parentWidget())->height());
  	}
  }else{
    nop->raise();
    nop->resize((nop->parentWidget())->width(), (nop->parentWidget())->height());
  }
}

void ProjectOptionsDlg::slotButtonOkClicked(){
	slotButtonApplyClicked();
	reject();
}

void ProjectOptionsDlg::slotButtonApplyClicked(){
	// iterate the compilers
	QListViewItem* compilerItem;
	QList<KDevCompiler> *compilers = m_ps->compilers();
  QListIterator<KDevCompiler> it(*compilers);
  for ( ; it.current(); ++it ) {
    KDevCompiler *comp = it.current();
		emit(ButtonApplyClicked(comp));
  }
}

#include "ProjectOptionsDlg.moc"
