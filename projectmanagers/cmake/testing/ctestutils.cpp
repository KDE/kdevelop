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

#include <interfaces/iproject.h>
#include <interfaces/itestcontroller.h>
#include <interfaces/icore.h>
#include <interfaces/iplugincontroller.h>
#include <interfaces/iruncontroller.h>
#include <interfaces/iprojectcontroller.h>
#include <project/interfaces/ibuildsystemmanager.h>
#include <project/projectmodel.h>
#include <language/duchain/indexedstring.h>

using namespace KDevelop;

void CTestUtils::createTestSuites(const QList< Test >& testSuites, const KUrl& path)
{
    ProjectFolderItem* folder = ICore::self()->projectController()->projectModel()->itemForUrl(IndexedString(path))->folder();
    if(!folder)
        return;

    IProject* project = folder->project();
    QString binDir = project->buildSystemManager()->buildDirectory(project->projectItem()).toLocalFile();
    KUrl currentBinDir = project->buildSystemManager()->buildDirectory(folder);
    KUrl currentSourceDir = folder->url();
    
    foreach (const Test& test, testSuites)
    {
        QString exe = test.executable;
        if (test.isTarget)
        {
            QList<ProjectTargetItem*> items = project->buildSystemManager()->targets(folder);
            foreach (ProjectTargetItem* item, items)
            {
                ProjectExecutableTargetItem * exeTgt = item->executable();
                if (exeTgt == 0)
                {
                    continue;
                }
                if (exeTgt->text() == test.executable)
                {
                    exe = exeTgt->builtUrl().toLocalFile();
                    kDebug(9042) << "Found proper test target path" << test.executable << "->" << exe;
                    break;
                }
            }
        }
        exe.replace("#[bin_dir]", binDir);
        KUrl exeUrl = KUrl(exe);
        if (exeUrl.isRelative())
        {
            exeUrl = currentBinDir;
            exeUrl.addPath(test.executable);
        }
        
        QStringList files;
        foreach (const QString& file, test.files)
        {
            KUrl fileUrl(file);
            if (fileUrl.isRelative())
            {
                fileUrl = currentSourceDir;
                fileUrl.addPath(file);
            }

            QString localFile = fileUrl.toLocalFile();
            localFile.replace("#[bin_dir]", binDir);
            files << localFile;
        }
        
        
        QStringList args = test.arguments;
        for (QStringList::iterator it = args.begin(); it != args.end(); ++it)
        {
            (*it).replace("#[bin_dir]", binDir);
        }
        
        CTestSuite* suite = new CTestSuite(test.name, exeUrl, files, project, args, test.properties.value("WILL_FAIL", "FALSE") == "TRUE");
        ICore::self()->runController()->registerJob(new CTestFindJob(suite));
    }
}
