/***************************************************************************
                          ProjectSpaceInfoWidget.cpp  -  description
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

#include "ProjectSpaceInfoWidget.h"
#include "projectspace.h"
#include <qlineedit.h>
#include <qmultilineedit.h>

ProjectSpaceInfoWidget::ProjectSpaceInfoWidget(QWidget *parent, const char *name, ProjectSpace* ps, QWidget* pdlg)
  :	ProjectSpaceInfoWidgetBase(parent,name){
	connect(pdlg, SIGNAL(ButtonApplyClicked()), this, SLOT(slotButtonApplyClicked()));
	m_pProjectSpace = ps;
	projectSpaceName->setText(m_pProjectSpace->name());
//	version->setText(m_pProjectSpace->version()); forgotten in projectspace!
	author->setText(m_pProjectSpace->author());
	initials->setText(m_pProjectSpace->initials());
	email->setText(m_pProjectSpace->email());
	company->setText(m_pProjectSpace->company());
//	info->setText(m_pProjectSpace->info()); forgotten in projectspace
}

ProjectSpaceInfoWidget::~ProjectSpaceInfoWidget(){
}

// writes the corrent project info
void ProjectSpaceInfoWidget::slotButtonApplyClicked(){
	m_pProjectSpace->setName(projectSpaceName->text());
	m_pProjectSpace->setVersion(version->text());
	m_pProjectSpace->setAuthor(author->text());
	m_pProjectSpace->setInitials(initials->text());
	m_pProjectSpace->setEmail(email->text());
	m_pProjectSpace->setCompany(company->text());
//	m_pProjectSpace->setInfoString(info->text());
}

#include "ProjectSpaceInfoWidget.moc"
