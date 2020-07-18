/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright (C) 2011 Martin Heide <martin.heide@gmx.net>                *
 *   Copyright (C) 2011 Julien Desgats <julien.desgats@gmail.com>          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#include "qmakebuilddirchooserdialog.h"

#include <debug.h>
#include "qmakebuilddirchooser.h"
#include "qmakeconfig.h"

#include <QDialogButtonBox>
#include <KLocalizedString>

#include <interfaces/iproject.h>

QMakeBuildDirChooserDialog::QMakeBuildDirChooserDialog(KDevelop::IProject* project, QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(i18nc("@title:window", "Configure a Build Directory"));

    auto mainWidget = new QWidget(this);
    auto mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    mainLayout->addWidget(mainWidget);

    m_chooserUi = new QMakeBuildDirChooser(project);
    connect(m_chooserUi, &QMakeBuildDirChooser::changed, this, &QMakeBuildDirChooserDialog::validate);
    mainLayout->addWidget(m_chooserUi);

    m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    m_buttonBox->button(QDialogButtonBox::Ok)->setDefault(true);
    connect(m_buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(m_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    mainLayout->addWidget(m_buttonBox);

    loadConfig();
    // save; like this, we can be sure to have a qmake executable and build path set
    //(even if user clicks Cancel)
    saveConfig();

    validate();
}

QMakeBuildDirChooserDialog::~QMakeBuildDirChooserDialog()
{
}

void QMakeBuildDirChooserDialog::loadConfig()
{
    m_chooserUi->loadConfig();
}

void QMakeBuildDirChooserDialog::saveConfig()
{
    // store this builds config
    m_chooserUi->saveConfig();

    // also save as current values
    KConfigGroup config(m_chooserUi->project()->projectConfiguration(), QMakeConfig::CONFIG_GROUP);
    m_chooserUi->saveConfig(config);
    config.writeEntry(QMakeConfig::BUILD_FOLDER, buildDir());
}

QString QMakeBuildDirChooserDialog::buildDir() const
{
    return m_chooserUi->buildDir();
}

void QMakeBuildDirChooserDialog::accept()
{
    if (m_chooserUi->validate()) {
        QDialog::accept();
        saveConfig();
    }
}

void QMakeBuildDirChooserDialog::validate()
{
    m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(m_chooserUi->validate());
}
