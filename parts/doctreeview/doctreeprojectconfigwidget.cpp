/***************************************************************************
 *   Copyright (C) 2002 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *   Copyright (C) 2002 by Sebastian Kratzert                              *
 *   skratzert@gmx.de                                                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "doctreeprojectconfigwidget.h"

#include <kurlrequester.h>
#include <kfiledialog.h>
#include "doctreeviewpart.h"
#include "doctreeviewwidget.h"
#include "kdevproject.h"
#include "domutil.h"


DocTreeProjectConfigWidget::DocTreeProjectConfigWidget(DocTreeViewWidget *widget, 
    QWidget *parent, KDevProject *project, const char *name): DocTreeProjectConfigWidgetBase(parent, name)

{
    m_widget = widget;
    m_project = project;
    readConfig();
}

/*
DocTreeProjectConfigWidget::~DocTreeProjectConfigWidget()
{}
*/

void DocTreeProjectConfigWidget::readConfig()
{
    QString userdocDir = DomUtil::readEntry(
        *m_project->projectDom() , 
        "/kdevdoctreeview/projectdoc/userdocDir", m_project->projectDirectory() + "/html/" );
    userdocdirEdit->setURL( userdocDir );
    userdocdirEdit->fileDialog()->setMode( KFile::Directory );
    
    QString apidocDir = DomUtil::readEntry(
        *m_project->projectDom() , 
        "/kdevdoctreeview/projectdoc/apidocDir", m_project->projectDirectory() + "/html/" );
    apidocdirEdit->setURL( userdocDir );
    apidocdirEdit->fileDialog()->setMode( KFile::Directory );
}

void DocTreeProjectConfigWidget::storeConfig()
{
    DomUtil::writeEntry(*m_project->projectDom(), 
        "/kdevdoctreeview/projectdoc/userdocDir", userdocdirEdit->url());
    DomUtil::writeEntry(*m_project->projectDom(), 
        "/kdevdoctreeview/projectdoc/apidocDir", apidocdirEdit->url());
}

void DocTreeProjectConfigWidget::accept()
{
    storeConfig();
    m_widget->configurationChanged();
}

/*
void DocTreeProjectConfigWidget::setProject(KDevProject* project)
{
    readConfig();
    m_project = project;
}

*/
