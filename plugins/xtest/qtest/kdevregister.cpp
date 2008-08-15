/*
* KDevelop xUnit integration
* Copyright 2008 Manuel Breugelmans <mbr.nxi@gmail.com>
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
* 02110-1301, USA.
*/

#include "kdevregister.h"
#include "documentaccess.h"
#include "suitebuilder.h"
#include <veritas/test.h>
#include "config/qtestsettings.h"
#include <interfaces/iproject.h>
#include <project/projectmodel.h>
#include <project/interfaces/ibuildsystemmanager.h>
#include <interfaces/iruncontroller.h>
#include <interfaces/icore.h>
#include <KDebug>
#include <QDir>

using QTest::KDevRegister;
using QTest::DocumentAccess;
using QTest::SuiteBuilder;
using QTest::Settings;
using Veritas::Test;
using namespace KDevelop;

namespace
{

/*! Recurses down the project item tree and collects all TestTargets.
    Returns these as a list. */
QList<ProjectTestTargetItem*> fetchAllTestTargets(ProjectBaseItem* root)
{
    QList<ProjectTestTargetItem*> tests;
    if (!root) return tests;
    foreach(ProjectTestTargetItem* t, root->testList()) {
        kDebug() << "Found Test " << t->data(Qt::DisplayRole).toString();
        tests << t;
    }
    foreach(ProjectFolderItem* fldr, root->folderList()) {
        tests += fetchAllTestTargets(fldr);
    }
    return tests;
}

/*! Get the test names from a list of TestTargetItems */
QStringList namesFromTargets(QList<ProjectTestTargetItem*> testTargets)
{
    QStringList testNames;
    foreach(ProjectTestTargetItem* test, testTargets) {
        testNames << test->data(Qt::DisplayRole).toString();
    }
    return testNames;
}

/*! Debug info printer */
void printFilesInTargets(QList<ProjectTestTargetItem*> targets)
{
    foreach(ProjectTestTargetItem* test, targets) {
        kDebug() << test->data(Qt::DisplayRole);
        foreach(ProjectFileItem* f, test->fileList()) {
            kDebug() << "- " << f->url();
        }
    }
}

/*! Locate all '.shell' files in @p dir for which the name is contained in
@p testNames. This function recurses down @p dir and its subdirectories.
Return the found test-shell scripts as a list of  QFileInfo's. */
QFileInfoList findTestShellFilesIn(QDir& dir, const QStringList& testNames)
{
    kDebug() << dir.absolutePath();
    QFileInfoList testShellFiles;
    QDir current(dir);
    current.setFilter(QDir::Dirs | QDir::NoDotAndDotDot | QDir::Readable | QDir::Writable);
    QStringList subDirs = current.entryList();
    foreach(QString subDir, subDirs) {
        current.cd(subDir);
        testShellFiles += findTestShellFilesIn(current, testNames);
        current.cdUp();
    }
    current = QDir(dir);
    current.setNameFilters(QStringList() << "*.shell");
    current.setFilter(QDir::Files |  QDir::Writable | QDir::NoSymLinks);
    QFileInfoList shellFiles = current.entryInfoList();
    foreach (QFileInfo s, shellFiles) {
        if (testNames.contains(s.baseName())) {
            testShellFiles << s;
        }
    }
    return testShellFiles;
}

QList<KUrl> fileInfo2KUrl(const QFileInfoList& fileInfos)
{
    QList<KUrl> urls;
    foreach(QFileInfo fi, fileInfos) {
        urls << KUrl(fi.absoluteFilePath());
    }
    return urls;
}

} // end anonymous namespace


KDevRegister::KDevRegister()
    : m_root(0)
{}

KDevRegister::~KDevRegister()
{}

QList<ProjectTestTargetItem*> KDevRegister::fetchTestTargets()
{
    return fetchAllTestTargets(project()->projectItem());
}

#define STOP_IF(X, MSG) \
if (X) {\
    m_root = new Test("");\
    return; \
} else void(0)

void KDevRegister::reload()
{
    Q_ASSERT(project());
    QList<ProjectTestTargetItem*> testTargets = fetchTestTargets();
    QList<QString> testNames = namesFromTargets(testTargets);

    IBuildSystemManager* bm = project()->buildSystemManager();
    STOP_IF(!bm, "Build system manager zero");

    KUrl buildRoot = bm->buildDirectory(project()->projectItem());
    STOP_IF(buildRoot.isEmpty(), "Root build directory empty");
    STOP_IF(buildRoot == KUrl("/./"), "Root build directory empty");

    QDir buildDir(buildRoot.path());
    QFileInfoList shells = findTestShellFilesIn(buildDir, testNames);
    foreach(QFileInfo shell, shells) {
        kDebug() << "shell -> " << shell.absoluteFilePath();
    }
    STOP_IF(shells.isEmpty(), "No test shell exes found.");

    QList<KUrl> shellUrls = fileInfo2KUrl(shells);
    SuiteBuilder sb;
    sb.setTestExecutables(shellUrls);
    sb.setSettings(settings());
    sb.start(); // TODO register job with runcontroller & be asynchronous
                // also move the file finding stuff in here.
    m_root = sb.root();
}

Test* KDevRegister::root() const
{
    Q_ASSERT(m_root);
    return m_root;
}


