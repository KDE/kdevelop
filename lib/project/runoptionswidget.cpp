/***************************************************************************
 *   Copyright (C) 2001-2002 by Bernd Gehrmann                             *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "runoptionswidget.h"

#include <klocale.h>
#include <kfiledialog.h>

#include <qlineedit.h>
#include <qlistview.h>
#include <qgroupbox.h>
#include <qcheckbox.h>

#include "domutil.h"
#include "environmentvariableswidget.h"


RunOptionsWidget::RunOptionsWidget(QDomDocument &dom, const QString &configGroup,
                                    const QString &projectDirectory,QWidget *parent, const char *name)
    : RunOptionsWidgetBase(parent, name),
      m_dom(dom), m_configGroup(configGroup)
{
    env_var_group->setColumnLayout( 1, Qt::Vertical );
    m_environmentVariablesWidget = new EnvironmentVariablesWidget( dom, configGroup + "/run/envvars", env_var_group );

    m_projectDirectory = projectDirectory;
    mainprogram_edit->setText(DomUtil::readEntry(dom, configGroup + "/run/mainprogram"));
    progargs_edit->setText(DomUtil::readEntry(dom, configGroup + "/run/programargs"));
    startinterminal_box->setChecked(DomUtil::readBoolEntry(dom, configGroup + "/run/terminal"));
}


RunOptionsWidget::~RunOptionsWidget()
{}


void RunOptionsWidget::accept()
{
    DomUtil::writeEntry(m_dom, m_configGroup + "/run/mainprogram", mainprogram_edit->text());
    DomUtil::writeEntry(m_dom, m_configGroup + "/run/programargs", progargs_edit->text());
    DomUtil::writeBoolEntry(m_dom, m_configGroup + "/run/terminal", startinterminal_box->isChecked());

    m_environmentVariablesWidget->accept();
}


void RunOptionsWidget::browseMainProgram()
{
  QString path = KFileDialog::getOpenFileName(m_projectDirectory,
                               i18n("*|All Files"),
                               this,
                               i18n("Select main program executable."));

  if (path.find(m_projectDirectory)==0)
  {
    path.remove(0,m_projectDirectory.length()+1);
    path = "./" + path;
  }
  mainprogram_edit->setText(path);
}

#include "runoptionswidget.moc"
