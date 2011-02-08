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

#include "qmakebuilddirchooser.h"
#include "qmakeconfig.h"
#include <KStandardDirs>
#include <KDebug>
#include <project/interfaces/ibuildsystemmanager.h>
#include <project/interfaces/iprojectbuilder.h>
#include <interfaces/icore.h>
#include <interfaces/iruncontroller.h>

QMakeBuildDirChooser::QMakeBuildDirChooser(QWidget *parent, KDevelop::IProject* project)
    :  Ui::QMakeBuildDirChooser(), m_project(project)
{
    setupUi(parent);
    status->setText("");
}

QMakeBuildDirChooser::~QMakeBuildDirChooser()
{
    //don't save in destructor; instead, on click of OK-button
    //saveConfig();
}


void QMakeBuildDirChooser::saveConfig()
{
    kDebug() << "Writing config";
    KConfigGroup cg(m_project->projectConfiguration(), QMakeConfig::CONFIG_GROUP);
    cg.writeEntry<KUrl>(QMakeConfig::QMAKE_BINARY, qmakeBin());
    cg.writeEntry<KUrl>(QMakeConfig::BUILD_FOLDER, buildDir());
    cg.writeEntry<KUrl>(QMakeConfig::INSTALL_PREFIX, installPrefix());
    cg.writeEntry(QMakeConfig::EXTRA_ARGUMENTS, extraArgs());
    cg.writeEntry<int>(QMakeConfig::BUILD_TYPE, buildType());
    cg.sync();
}

void QMakeBuildDirChooser::loadConfig()
{
    KConfigGroup cg(m_project->projectConfiguration(), QMakeConfig::CONFIG_GROUP);
    setQmakeBin( cg.readEntry<KUrl>(QMakeConfig::QMAKE_BINARY, KUrl(KStandardDirs::findExe("qmake"))) );
    KUrl proposedBuildUrl( m_project->folder().toLocalFile() + "/build" );
    proposedBuildUrl.cleanPath();
    setBuildDir( cg.readEntry<KUrl>(QMakeConfig::BUILD_FOLDER, proposedBuildUrl) );
    setInstallPrefix( cg.readEntry<KUrl>(QMakeConfig::INSTALL_PREFIX, KUrl("")) );
    setExtraArgs( cg.readEntry(QMakeConfig::EXTRA_ARGUMENTS) );
    setBuildType( cg.readEntry<int>(QMakeConfig::BUILD_TYPE, 0) );
    status->setText("");
}


bool QMakeBuildDirChooser::isValid()
{
    bool valid = true;
    if(qmakeBin().isEmpty())
    {
        status->setText(i18n("Please specify path to QMake binary"));
        valid = false;
    }
    else if(!qmakeBin().isValid())
    {
        status->setText(i18n("QMake binary path is invalid"));
        valid = false;
    }
    else if(!qmakeBin().isLocalFile())
    {
        status->setText(i18n("QMake binary must be a local path"));
        valid = false;        
    }
    
    if(buildDir().isEmpty())
    {
        status->setText(i18n("Please specify a build folder"));
        valid = false;
    }
    else if(!buildDir().isValid())
    {
        status->setText(i18n("Build folder is invalid"));
        valid = false;
    }
    else if(!buildDir().isLocalFile())
    {
        status->setText(i18n("Build folder must be a local path"));
        valid = false;
    }

    if(!installPrefix().isEmpty() && !installPrefix().isValid())
    {
        status->setText(i18n("Install prefix is invalid (may also be left empty)"));
        valid = false;
    }
    if(!installPrefix().isEmpty() && !installPrefix().isLocalFile())
    {
        status->setText(i18n("Install prefix must be a local path (may also be left empty)"));
        valid = false;        
    }
    
    if(valid)
        status->setText("");
    kDebug() << "VALID == " << valid;
    return valid;
}

KUrl QMakeBuildDirChooser::qmakeBin() const
{
    return kcfg_qmakeBin->url();
}

KUrl QMakeBuildDirChooser::buildDir() const
{
    return kcfg_buildDir->url();
}

KUrl QMakeBuildDirChooser::installPrefix() const
{
    return kcfg_installPrefix->url();
}

int QMakeBuildDirChooser::buildType() const
{
    return kcfg_buildType->currentIndex();
}
QString QMakeBuildDirChooser::extraArgs() const
{
    return kcfg_extraArgs->text();
}

void QMakeBuildDirChooser::setQmakeBin(const KUrl& url)
{
    kcfg_qmakeBin->setUrl(url);
}

void QMakeBuildDirChooser::setBuildDir(const KUrl& url)
{
    kcfg_buildDir->setUrl(url);
}

void QMakeBuildDirChooser::setInstallPrefix(const KUrl& url)
{
    kcfg_installPrefix->setUrl(url);
}

void QMakeBuildDirChooser::setBuildType(int type)
{
    kcfg_buildType->setCurrentIndex(type);
}

void QMakeBuildDirChooser::setExtraArgs(const QString& args)
{
    kcfg_extraArgs->setText(args);
}
