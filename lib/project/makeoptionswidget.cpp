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

#include "makeoptionswidget.h"

#include <qcheckbox.h>
#include <qlineedit.h>
#include <qspinbox.h>
#include <qlistview.h>
#include <qgroupbox.h>

#include "domutil.h"
#include "environmentvariableswidget.h"

MakeOptionsWidget::MakeOptionsWidget(QDomDocument &dom, const QString &configGroup,
                                   QWidget *parent, const char *name)
    : MakeOptionsWidgetBase(parent, name),
      m_dom(dom), m_configGroup(configGroup)
{
    env_var_group->setColumnLayout( 1, Qt::Vertical );
    m_environmentVariablesWidget = new EnvironmentVariablesWidget( dom, configGroup, env_var_group );

    abort_box->setChecked(DomUtil::readBoolEntry(dom, configGroup + "/make/abortonerror"));
    jobs_box->setValue(DomUtil::readIntEntry(dom, configGroup + "/make/numberofjobs"));
    dontact_box->setChecked(DomUtil::readBoolEntry(dom, configGroup + "/make/dontact"));
    makebin_edit->setText(DomUtil::readEntry(dom, configGroup + "/make/makebin"));
}


MakeOptionsWidget::~MakeOptionsWidget()
{}


void MakeOptionsWidget::accept()
{
    DomUtil::writeBoolEntry(m_dom, m_configGroup + "/make/abortonerror", abort_box->isChecked());
    DomUtil::writeIntEntry(m_dom, m_configGroup + "/make/numberofjobs", jobs_box->value());
    DomUtil::writeBoolEntry(m_dom, m_configGroup + "/make/dontact", dontact_box->isChecked());
    DomUtil::writeEntry(m_dom, m_configGroup + "/make/makebin", makebin_edit->text());

    m_environmentVariablesWidget->accept();
}

#include "makeoptionswidget.moc"
