/***************************************************************************
 *   Copyright (C) 2003 by Hendrik Kueck                                   *
 *   kueck@cs.ubc.ca                                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "custommakeconfigwidget.h"
#include <custombuildoptionswidgetbase.h>
#include <customprojectpart.h>
#include <qcombobox.h>
#include <qdir.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qspinbox.h>
#include <qlistview.h>
#include <qgroupbox.h>
#include <qvalidator.h>
#include <kdebug.h>
#include <klocale.h>

#include <environmentvariableswidget.h>

CustomMakeConfigWidget::CustomMakeConfigWidget(CustomProjectPart* part, const QString& configGroup, QWidget* parent)
 : CustomMakeConfigWidgetBase(parent),
    m_part(part), m_configGroup(configGroup), m_dom( *part->projectDom() )
{
    abort_box->setChecked(DomUtil::readBoolEntry(m_dom, m_configGroup + "/make/abortonerror"));
    jobs_box->setValue(DomUtil::readIntEntry(m_dom, m_configGroup + "/make/numberofjobs"));
    prio_box->setValue(DomUtil::readIntEntry(m_dom, m_configGroup + "/make/prio"));
    dontact_box->setChecked(DomUtil::readBoolEntry(m_dom, m_configGroup + "/make/dontact"));
    makebin_edit->setText(DomUtil::readEntry(m_dom, m_configGroup + "/make/makebin"));
    makeoptions_edit->setText(DomUtil::readEntry(m_dom, m_configGroup + "/make/makeoptions"));

    envs_combo->setValidator(new QRegExpValidator(QRegExp("^\\D.*"), this));
    m_allEnvironments = m_part->allMakeEnvironments();
    m_currentEnvironment = m_part->currentMakeEnvironment();
    env_var_group->setColumnLayout( 1, Qt::Vertical );
    m_envWidget = new EnvironmentVariablesWidget(m_dom, m_configGroup + "/make/environments/" + m_currentEnvironment, env_var_group);
    envs_combo->insertStringList(m_allEnvironments);
    envs_combo->setEditText(m_currentEnvironment);
}


CustomMakeConfigWidget::~CustomMakeConfigWidget()
{

}

void CustomMakeConfigWidget::envNameChanged(const QString& envName)
{
    QStringList allEnvNames = m_part->allMakeEnvironments();
    bool canAdd = !allEnvNames.contains(envName) && !envName.contains("/") && !envName.isEmpty();
    bool canRemove = allEnvNames.contains(envName) && allEnvNames.count() > 1;
    addenvs_button->setEnabled(canAdd);
    copyenvs_button->setEnabled(canAdd);
    removeenvs_button->setEnabled(canRemove);
}

void CustomMakeConfigWidget::envChanged(const QString& envName)
{
    if (envName == m_currentEnvironment || !m_allEnvironments.contains(envName))
        return;

    // save settings of previously active environment
    if (!m_currentEnvironment.isNull() )
        m_envWidget->accept();

    m_currentEnvironment = envName;
    m_envWidget->readEnvironment(m_dom, m_configGroup + "/make/environments/" + envName);
    envs_combo->setEditText(envName);
}

void CustomMakeConfigWidget::envAdded()
{
    QString env = envs_combo->currentText();
    m_allEnvironments.append(env);

    envs_combo->clear();
    envs_combo->insertStringList(m_allEnvironments);
    envChanged(env);
}

void CustomMakeConfigWidget::envRemoved()
{
    QString env = envs_combo->currentText();
    QDomNode node = DomUtil::elementByPath(m_dom, m_configGroup + "/make/environments");
    node.removeChild(node.namedItem(env));
    m_allEnvironments.remove(env);
    envs_combo->clear();
    envs_combo->insertStringList(m_allEnvironments);
    m_currentEnvironment = QString::null;
    envChanged( m_allEnvironments[0] );
}

void CustomMakeConfigWidget::envCopied()
{
    QString env = envs_combo->currentText();
    m_allEnvironments.append(env);
    envs_combo->clear();
    envs_combo->insertStringList(m_allEnvironments);
    m_currentEnvironment = env;
    m_envWidget->changeConfigGroup(m_configGroup + "/make/environments/" + env);
    envs_combo->setEditText(env);
}

void CustomMakeConfigWidget::accept()
{
    DomUtil::writeBoolEntry(m_dom, m_configGroup + "/make/abortonerror", abort_box->isChecked());
    DomUtil::writeIntEntry(m_dom, m_configGroup + "/make/numberofjobs", jobs_box->value());
    DomUtil::writeIntEntry(m_dom, m_configGroup + "/make/prio", prio_box->value());
    DomUtil::writeBoolEntry(m_dom, m_configGroup + "/make/dontact", dontact_box->isChecked());
    DomUtil::writeEntry(m_dom, m_configGroup + "/make/makebin", makebin_edit->text());
    DomUtil::writeEntry(m_dom, m_configGroup + "/make/makeoptions", makeoptions_edit->text());
    DomUtil::writeEntry(m_dom, m_configGroup + "/make/selectedenvironment", m_currentEnvironment);
    m_envWidget->accept();
}

#include "custommakeconfigwidget.moc"
