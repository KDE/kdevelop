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
#include <KDebug>
#include "qmakeconfig.h"

#include <interfaces/iproject.h>

QMakeBuildDirChooserDialog::QMakeBuildDirChooserDialog(KDevelop::IProject* project, QWidget* parent) :
    KDialog(parent),
    QMakeBuildDirChooser(mainWidget(), project)
{
    setButtons( KDialog::Ok | KDialog::Cancel );
    setCaption( i18n("Configure QMake build settings") );
    setDefaultButton( KDialog::Ok );

    connect(kcfg_qmakeBin, SIGNAL(textChanged(QString)), this, SLOT(validate()));
    connect(kcfg_buildDir, SIGNAL(textChanged(QString)), this, SLOT(validate()));
    connect(kcfg_installPrefix, SIGNAL(textChanged(QString)), this, SLOT(validate()));
    //connect(extraArguments, SIGNAL(textChanged(QString)), this, SLOT(validate()));

    loadConfig();
    //save; like this, we can be sure to have a qmake binary and build path set
    //(even if user clicks Cancel)
    saveConfig();

    validate();
}

QMakeBuildDirChooserDialog::~QMakeBuildDirChooserDialog()
{

}

void QMakeBuildDirChooserDialog::saveConfig()
{
    // store this builds config
    QMakeBuildDirChooser::saveConfig();

    // also save as current values
    KConfigGroup config(m_project->projectConfiguration(), QMakeConfig::CONFIG_GROUP);
    QMakeBuildDirChooser::saveConfig(config);
    config.writeEntry(QMakeConfig::BUILD_FOLDER, buildDir());
}


void QMakeBuildDirChooserDialog::slotButtonClicked(int button)
{
    if(button == KDialog::Ok)
    {
        if(isValid())
        {
            accept();
            saveConfig();
        }
        else
        {
            kDebug() << "OK-button not accepted, input invalid";
        }
    }
    else
    {
        kDebug() << "button != OK";
        KDialog::slotButtonClicked(button);
    }
}

void QMakeBuildDirChooserDialog::validate()
{
    button(KDialog::Ok)->setEnabled(isValid());
}

