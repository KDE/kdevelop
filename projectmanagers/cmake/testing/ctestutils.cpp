/*  This file is part of KDevelop
    Copyright 2012 Miha Čančula <miha@noughmad.eu>

    CTestTestfile.cmake parsing uses code from the xUnit plugin
    Copyright 2008 Manuel Breugelmans <mbr.nxi@gmail.com>
    Copyright 2010 Daniel Calviño Sánchez <danxuliu@gmail.com>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; see the file COPYING.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "ctestutils.h"
#include "ctestsuite.h"
#include "ctestfindjob.h"
#include "parser/cmakelistsparser.h"
#include "../debug.h"

#include <interfaces/iproject.h>
#include <interfaces/itestcontroller.h>
#include <interfaces/icore.h>
#include <interfaces/iplugincontroller.h>
#include <interfaces/iruncontroller.h>
#include <interfaces/iprojectcontroller.h>
#include <project/interfaces/ibuildsystemmanager.h>
#include <project/projectmodel.h>
#include <QFileInfo>

using namespace KDevelop;

namespace
{

QList<Path> filesToUrls(const QList<ProjectFileItem*>& files)
{
    QList<Path> ret;
    for(ProjectFileItem* f : files) {
        ret += f->path();
    }
    return ret;
}

QList<Path> filesForExecutable(ProjectFolderItem* folder, const QUrl& builtUrl)
{
    for(ProjectTargetItem* target : folder->targetList()) {
        ProjectExecutableTargetItem* exe = target->executable();
        if (exe && exe->builtUrl() == builtUrl) {
            return filesToUrls(exe->fileList());
        }
    }
    for(ProjectFolderItem* f : folder->folderList()) {
        QList<Path> urls = filesForExecutable(f, builtUrl);
        if (!urls.isEmpty())
            return urls;
    }
    return QList<Path>();
}

}

void CTestUtils::createTestSuites(ProjectFolderItem* folder)
{
    IProject* project = folder->project();
    IBuildSystemManager* bsm = project->buildSystemManager();
    const QString binDir = bsm->buildDirectory(folder->project()->projectItem()).toLocalFile();
    const Path currentBinDir = bsm->buildDirectory(folder);
    const Path currentSourceDir = folder->path();

    CMakeFileContent fileContents = CMakeListsParser::readCMakeFile(currentBinDir.toLocalFile()+"/CTestTestfile.cmake");
    for(const CMakeFunctionDesc& func : fileContents)
    {
        if (func.name.compare("add_test", Qt::CaseInsensitive) != 0)
            continue;
        QStringList args = func.argsList();
        QString testName = args.takeFirst();
        QUrl exePath = QUrl::fromLocalFile(args.takeFirst());

        CTestSuite* suite = new CTestSuite(testName, exePath, filesForExecutable(folder, exePath), project, args, false /*expect fail*/);
        ICore::self()->runController()->registerJob(new CTestFindJob(suite));
    }

    for(ProjectFolderItem* item : folder->folderList()) {
        createTestSuites(item);
    }
}
