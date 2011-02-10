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
    kcfg_buildDir->setMode(KFile::Directory | KFile::LocalOnly);
    kcfg_installPrefix->setMode(KFile::Directory);
    kcfg_qmakeBin->setMode(KFile::File | KFile::ExistingOnly | KFile::LocalOnly);
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

    // Write entries to builds group
    // current entries are handled by KConfig in config dialog, import dialog handles it itself
    KConfigGroup currentBuild = cg.group(buildDir().toLocalFile());
    currentBuild.writeEntry(QMakeConfig::QMAKE_BINARY, qmakeBin());
    currentBuild.writeEntry(QMakeConfig::INSTALL_PREFIX, installPrefix());
    currentBuild.writeEntry(QMakeConfig::EXTRA_ARGUMENTS, extraArgs());
    currentBuild.writeEntry(QMakeConfig::BUILD_TYPE, buildType());
    currentBuild.sync();
    cg.sync();
}

void QMakeBuildDirChooser::loadConfig()
{
    KUrl proposedBuildUrl( m_project->folder().toLocalFile() + "/build" );
    proposedBuildUrl.cleanPath();
    KConfigGroup cg(m_project->projectConfiguration(), QMakeConfig::CONFIG_GROUP);
    loadConfig(cg.readEntry<KUrl>(QMakeConfig::BUILD_FOLDER, proposedBuildUrl).toLocalFile());
}

void QMakeBuildDirChooser::loadConfig(const QString& config)
{
    // get correct group
    KConfigGroup cg(m_project->projectConfiguration(), QMakeConfig::CONFIG_GROUP);
    const KConfigGroup build = cg.group(config);

    // sets values into fields
    setQmakeBin( build.readEntry<KUrl>(QMakeConfig::QMAKE_BINARY, KUrl(KStandardDirs::findExe("qmake"))) );
    setBuildDir( KUrl(config) );
    setInstallPrefix( build.readEntry<KUrl>(QMakeConfig::INSTALL_PREFIX, KUrl("")) );
    setExtraArgs( build.readEntry<QString>(QMakeConfig::EXTRA_ARGUMENTS, ""));
    setBuildType( build.readEntry<int>(QMakeConfig::BUILD_TYPE, 0) );
    status->setText("");
}

bool QMakeBuildDirChooser::isValid(QString *message)
{
    bool valid = true;
    QString msg;
    if(qmakeBin().isEmpty())
    {
        msg = i18n("Please specify path to QMake binary.");
        valid = false;
    }
    else if(!qmakeBin().isValid())
    {
        msg =  i18n("QMake binary path is invalid.");
        valid = false;
    }
    else if(!qmakeBin().isLocalFile())
    {
        msg = i18n("QMake binary must be a local path.");
        valid = false;
    }
    else
    {
        QFileInfo info(qmakeBin().toLocalFile());
        if(!info.isFile())
        {
            msg = i18n("QMake binary is not a file.");
            valid = false;
        }
        else if(!info.isExecutable())
        {
            msg = i18n("QMake binary is not executable.");
            valid = false;
        }
    }
    
    if(buildDir().isEmpty())
    {
        msg = i18n("Please specify a build folder.");
        valid = false;
    }
    else if(!buildDir().isValid())
    {
        msg = i18n("Build folder is invalid.");
        valid = false;
    }
    else if(!buildDir().isLocalFile())
    {
        msg = i18n("Build folder must be a local path.");
        valid = false;
    }

    if(!installPrefix().isEmpty() && !installPrefix().isValid())
    {
        msg = i18n("Install prefix is invalid (may also be left empty).");
        valid = false;
    }
    if(!installPrefix().isEmpty() && !installPrefix().isLocalFile())
    {
        msg = i18n("Install prefix must be a local path (may also be left empty).");
        valid = false;        
    }
    
    if (message)
    {
        *message = msg;
    }
    status->setText(msg);
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
