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
#include <kdebug.h>
#include <qheader.h>
#include <qlistview.h>
#include <kiconloader.h>
#include "ProjectInfoWidget.h"
#include "ProjectSpaceInfoWidget.h"
#include <qlayout.h>
#include <qobjectlist.h>
#include "klocale.h"
#include <qhbox.h>
#include <qvbox.h>
#include "KDevCompiler.h"
#include "KDevLinker.h"


/*
 * Author: Omid Givi
 * class ProjectOptionsDlg
 */

ProjectOptionsDlg::ProjectOptionsDlg(QWidget *parent, const char *name, ProjectSpace *ps)
    : KDialogBase(TreeList, i18n("Current Project Options"), Apply|Ok|Cancel, Ok,
                  parent, name){
  m_ps = ps;

  // Where do I get the right debug number for 'this'??
  kdDebug(9000) << "Opening ProjectOptions Dialog" << endl;

  QVBox *vbox;
	setShowIconsInTreeList(true);
	setRootIsDecorated(false);
	
	QWidget* optionsWidget;

	// Project info page
  vbox = addVBoxPage(i18n("Project"), i18n("Current Project Information"),
  									 KGlobal::iconLoader()->loadIcon("info", KIcon::Desktop, KIcon::SizeSmall));
	optionsWidget = new ProjectInfoWidget(vbox, "Current Project Information", m_ps, this);
	
	// ProjectSpace info page
  vbox = addVBoxPage(i18n("Project Space"), i18n("Project Space Information"),
  									 KGlobal::iconLoader()->loadIcon("info", KIcon::Desktop, KIcon::SizeSmall));
	optionsWidget = new ProjectSpaceInfoWidget(vbox, "Project Space Information", m_ps, this);
	
	
	// compilers
  vbox = addVBoxPage(i18n("Compilers"), i18n("Compilers Options"),
  									 KGlobal::iconLoader()->loadIcon("folder", KIcon::Desktop, KIcon::SizeSmall));
		
	// iterate the compilers
	QList<KDevCompiler> *compilers = ps->currentProject()->compilers();
  QListIterator<KDevCompiler> it(*compilers);
  for ( ; it.current(); ++it ) {
    KDevCompiler *comp = it.current();
    QStringList qsl = QStringList() << "Compilers" << *(comp->name());
	  vbox = addVBoxPage(qsl, i18n("Compiler Options for: ") + *(comp->name()),
  										 KGlobal::iconLoader()->loadIcon(*(comp->icon()), KIcon::Desktop, KIcon::SizeSmall));
		
   	optionsWidget = comp->initOptionsWidget(vbox, this);
  }

  // linker	
	KDevLinker* kdl = ps->currentProject()->linker();
  vbox = addVBoxPage(i18n("Linker"), i18n("Linker"),
  									 KGlobal::iconLoader()->loadIcon(*(kdl->icon()), KIcon::Desktop, KIcon::SizeSmall));
	
  optionsWidget = kdl->initOptionsWidget(vbox, this);
  emit(WidgetStarted());
}	

ProjectOptionsDlg::~ProjectOptionsDlg(){
}

void ProjectOptionsDlg::slotOk(){
	slotApply();
	accept();
}

void ProjectOptionsDlg::slotApply(){
	emit(ButtonApplyClicked());
}

#include "ProjectOptionsDlg.moc"
