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
#include "../debug.h"

#include <interfaces/iproject.h>
#include <interfaces/itestcontroller.h>
#include <interfaces/icore.h>
#include <interfaces/iplugincontroller.h>
#include <interfaces/iruncontroller.h>
#include <interfaces/iprojectcontroller.h>
#include <project/interfaces/ibuildsystemmanager.h>
#include <project/projectmodel.h>
#include <util/path.h>
#include <QFileInfo>

using namespace KDevelop;

#pragma message("TODO: we are lacking introspection into targets, to see what files belong to each target.")

void CTestUtils::createTestSuites(const QVector<Test>& testSuites, KDevelop::IProject* project)
{
//     IProject* project = folder->project();
//     IBuildSystemManager* bsm = project->buildSystemManager();
//     const QString binDir = bsm->buildDirectory(folder->project()->projectItem()).toLocalFile();
//     const Path currentBinDir = bsm->buildDirectory(folder);
//     const Path currentSourceDir = folder->path();
//     QList<ProjectTargetItem*> items = bsm->targets(folder);

    foreach (const Test& test, testSuites)
    {
        QList<KDevelop::Path> files;
//         QString targetName = QFileInfo(test.executable).fileName();
//         foreach (ProjectTargetItem* item, items)
//         {
//             ProjectExecutableTargetItem * exeTgt = item->executable();
//             if (exeTgt && (exeTgt->text() == test.executable || exeTgt->text()==targetName))
//             {
//                 exe = exeTgt->builtUrl().toLocalFile();
//                 qCDebug(CMAKE) << "Found proper test target path" << test.executable << "->" << exe;
//                 foreach(ProjectFileItem* file, exeTgt->fileList()) {
//                     files += file->url();
//                 }
//                 break;
//             }
//         }
//         exe.replace("#[bin_dir]", binDir);

        const bool willFail = test.properties.value("WILL_FAIL", "FALSE") == "TRUE";
        CTestSuite* suite = new CTestSuite(test.name, test.executable, files, project, test.arguments, willFail);
        ICore::self()->runController()->registerJob(new CTestFindJob(suite));
    }
}
