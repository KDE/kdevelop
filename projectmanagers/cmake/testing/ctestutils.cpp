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
#include <project/interfaces/ibuildsystemmanager.h>
#include <project/projectmodel.h>

using namespace KDevelop;

void CTestUtils::createTestSuites(const QList< Test >& testSuites, ProjectFolderItem* folder)
{
    QString binDir = folder->project()->buildSystemManager()->buildDirectory(folder->project()->projectItem()).toLocalFile();
    KUrl currentBinDir = folder->project()->buildSystemManager()->buildDirectory(folder);
    KUrl currentSourceDir = folder->url();
    
    foreach (const Test& test, testSuites)
    {
        QString exe = test.executable;
        if (test.isTarget)
        {
            QList<ProjectTargetItem*> items = folder->project()->buildSystemManager()->targets(folder);
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
                    kDebug(9042) << "Find proper test target path" << test.executable << "->" << exe;
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
        
        CTestSuite* suite = new CTestSuite(test.name, exeUrl, files, folder->project(), args, test.properties.value("WILL_FAIL", "FALSE") == "TRUE");
        ICore::self()->runController()->registerJob(new CTestFindJob(suite));
    }
}
