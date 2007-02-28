/***************************************************************************
 *   Copyright (C) 2005 by Achim Herwig                                    *
 *   achim.herwig@wodca.de                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "customotherconfigwidget.h"
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

CustomOtherConfigWidget::CustomOtherConfigWidget(CustomProjectPart* part, const QString& configGroup, QWidget* parent)
 : CustomOtherConfigWidgetBase(parent),
    m_part(part), m_configGroup(configGroup), m_dom( *part->projectDom() )
{
    prio_box->setValue(DomUtil::readIntEntry(m_dom, m_configGroup + "/other/prio"));
    makebin_edit->setText(DomUtil::readEntry(m_dom, m_configGroup + "/other/otherbin"));
    defaultTarget_edit->setText(DomUtil::readEntry(m_dom, m_configGroup + "/other/defaulttarget"));
    makeoptions_edit->setText(DomUtil::readEntry(m_dom, m_configGroup + "/other/otheroptions"));

    envs_combo->setValidator(new QRegExpValidator(QRegExp("^\\D.*"), this));
    m_allEnvironments = m_part->allMakeEnvironments();
    m_currentEnvironment = m_part->currentMakeEnvironment();
    env_var_group->setColumnLayout( 1, Qt::Vertical );
    m_envWidget = new EnvironmentVariablesWidget(m_dom, m_configGroup + "/other/environments/" + m_currentEnvironment, env_var_group);
    envs_combo->insertStringList(m_allEnvironments);
    envs_combo->setEditText(m_currentEnvironment);
}


CustomOtherConfigWidget::~CustomOtherConfigWidget()
{

}

void CustomOtherConfigWidget::envNameChanged(const QString& envName)
{
    QStringList allEnvNames = m_part->allMakeEnvironments();
    bool canAdd = !allEnvNames.contains(envName) && !envName.contains("/") && !envName.isEmpty();
    bool canRemove = allEnvNames.contains(envName) && allEnvNames.count() > 1;
    addenvs_button->setEnabled(canAdd);
    copyenvs_button->setEnabled(canAdd);
    removeenvs_button->setEnabled(canRemove);
}

void CustomOtherConfigWidget::envChanged(const QString& envName)
{
    if (envName == m_currentEnvironment || !m_allEnvironments.contains(envName))
        return;

    // save settings of previously active environment
    if (!m_currentEnvironment.isNull() )
        m_envWidget->accept();

    m_currentEnvironment = envName;
    m_envWidget->readEnvironment(m_dom, m_configGroup + "/other/environments/" + envName);
    envs_combo->setEditText(envName);
}

void CustomOtherConfigWidget::envAdded()
{
    QString env = envs_combo->currentText();
    m_allEnvironments.append(env);

    envs_combo->clear();
    envs_combo->insertStringList(m_allEnvironments);
    envChanged(env);
}

void CustomOtherConfigWidget::envRemoved()
{
    QString env = envs_combo->currentText();
    QDomNode node = DomUtil::elementByPath(m_dom, m_configGroup + "/other/environments");
    node.removeChild(node.namedItem(env));
    m_allEnvironments.remove(env);
    envs_combo->clear();
    envs_combo->insertStringList(m_allEnvironments);
    m_currentEnvironment = QString::null;
    envChanged( m_allEnvironments[0] );
}

void CustomOtherConfigWidget::envCopied()
{
    QString env = envs_combo->currentText();
    m_allEnvironments.append(env);
    envs_combo->clear();
    envs_combo->insertStringList(m_allEnvironments);
    m_currentEnvironment = env;
    m_envWidget->changeConfigGroup(m_configGroup + "/other/environments/" + env);
    envs_combo->setEditText(env);
}

void CustomOtherConfigWidget::accept()
{
    DomUtil::writeIntEntry(m_dom, m_configGroup + "/other/prio", prio_box->value());
    DomUtil::writeEntry(m_dom, m_configGroup + "/other/otherbin", makebin_edit->text());
    DomUtil::writeEntry(m_dom, m_configGroup + "/other/defaulttarget", defaultTarget_edit->text());
    DomUtil::writeEntry(m_dom, m_configGroup + "/other/otheroptions", makeoptions_edit->text());
    DomUtil::writeEntry(m_dom, m_configGroup + "/other/selectedenvironment", m_currentEnvironment);
    m_envWidget->accept();
}

#include "customotherconfigwidget.moc"
// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on

