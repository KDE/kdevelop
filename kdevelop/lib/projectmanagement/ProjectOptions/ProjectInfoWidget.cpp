/***************************************************************************
                          ProjectInfoWidget.cpp  -  description
                             -------------------
    begin                : Mon Mar 5 2001
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

#include "ProjectInfoWidget.h"
#include "projectspace.h"
#include <qlineedit.h>

ProjectInfoWidget::ProjectInfoWidget(QWidget *parent, const char *name, ProjectSpace* ps, QWidget* pdlg)
  :	ProjectInfoWidgetBase(parent,name){
	connect(pdlg, SIGNAL(ButtonApplyClicked()), this, SLOT(slotButtonApplyClicked()));
	m_pProjectSpace = ps;
	projectName->setText(m_pProjectSpace->currentProject()->name());
	version->setText(m_pProjectSpace->currentProject()->version());
	projectSpace->setText(m_pProjectSpace->name());
	path->setText(m_pProjectSpace->currentProject()->absolutePath());
	// TODO: info string & release, als other stuff project or projectspace needs
}

ProjectInfoWidget::~ProjectInfoWidget(){
}

// writes the corrent project info
void ProjectInfoWidget::slotButtonApplyClicked(){
	m_pProjectSpace->currentProject()->setName(projectName->text());
	m_pProjectSpace->currentProject()->setVersion(version->text());
	m_pProjectSpace->currentProject()->setAbsolutePath(path->text());
}

#include "ProjectInfoWidget.moc"
