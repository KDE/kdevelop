/***************************************************************************
 *   Copyright 2008 Manuel Breugelmans <mbr.nxi@gmail.com>                 *
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


#include "kde4addunittest.h"
#include "autotestshell.h"
#include <QtTest/QTest>
#include <qtest_kde.h>
#include <QTextStream>
#include <project/interfaces/iprojectfilemanager.h>
#include <project/projectmodel.h>
#include <interfaces/iproject.h>
#include <interfaces/iprojectcontroller.h>

#define PROJ_DIR_NAME "cmake_test_kde4_add_unit_test_dir"
#define PROJ_NAME "cmake_test_kde4_add_unit_test"

using namespace KDevelop;

namespace
{

void removeTempDir(const QString& dirName)
{
    QString dir_ = QDir::tempPath() + "/" + dirName;
    QDir dir(dir_);
    if (!dir.exists()) return;
    foreach(QString file, dir.entryList()) {
       dir.remove(file);
    }
    QDir::temp().rmdir(dir_);
}

KUrl writeKDevProjectFile(const QDir& projDir)
{
    KUrl configUrl = KUrl(projDir.absolutePath() + "/" PROJ_NAME ".kdev4");
    QFile f(configUrl.pathOrUrl());
    f.open(QIODevice::WriteOnly);
    QTextStream str(&f);
    str << "[Project]\n"
        << "Name=" << PROJ_NAME << "\n"
        << "Manager=KDevCMakeManager\n";
    f.close();    
    return configUrl;
}

void writeTempProjectFile(const QDir& projDir)
{
    QDir tmpProjectDir = QDir(projDir);
    tmpProjectDir.mkdir(".kdev4");
    tmpProjectDir.cd(".kdev4");
    KUrl configUrl = KUrl(tmpProjectDir.absolutePath() + "/" PROJ_NAME ".kdev4");
    QFile f(configUrl.pathOrUrl());
    f.open(QIODevice::WriteOnly);
    QTextStream str(&f);
    QString buildDir(projDir.absolutePath() + "/build/");
    str << "[CMake]\n"
           "BuildDirs[$e]=" << buildDir << "\n"
           "CurrentBuildDir=" << buildDir << "\n";
    f.close();    
}

void writeCMakeLists(const QDir& projDir)
{
    KUrl configUrl = KUrl(projDir.absolutePath() + "/CMakeLists.txt");
    QFile f(configUrl.pathOrUrl());
    f.open(QIODevice::WriteOnly);
    QTextStream str(&f);
    str << "enable_testing()\n"
        << "find_package(KDE4 REQUIRED)\n"
        << "kde4_add_unit_test(foo foo.cpp)\n";
    f.close();    
}

void writeFooCpp(const QDir& projDir)
{
    KUrl configUrl = KUrl(projDir.absolutePath() + "/foo.cpp");
    QFile f(configUrl.pathOrUrl());
    f.open(QIODevice::WriteOnly);
    QTextStream str(&f);
    str << "int main(int,char**) {return 0;}\n";
    f.close();    
}

QDir createProjectDir()
{
    removeTempDir(PROJ_DIR_NAME);
    QDir tmpProjectDir = QDir(QDir::tempPath());
    tmpProjectDir.mkdir(PROJ_DIR_NAME);
    tmpProjectDir.cd(PROJ_DIR_NAME);
    return tmpProjectDir;
}

} // end anonymous namespace

void KDE4AddUnitTest::initTestCase()
{
    AutoTestShell::init();
    Core::initialize();

    m_tmpProjectDir = createProjectDir();
    m_projectFile = writeKDevProjectFile(m_tmpProjectDir);
    writeTempProjectFile(m_tmpProjectDir); // set build dir in .kdev4/project.kdev4 
    writeCMakeLists(m_tmpProjectDir); // adds a single kde4_add_unit_test
    writeFooCpp(m_tmpProjectDir);
}

void KDE4AddUnitTest::cleanupTestCase()
{
    removeTempDir(PROJ_DIR_NAME);
}

#define WAIT_FOR_OPEN_SIGNAL \
{\
    bool gotSignal = QTest::kWaitForSignal(projCtrl, SIGNAL(projectOpened(KDevelop::IProject*)), 2000);\
    QVERIFY2(gotSignal, "Timeout while waiting for opened signal");\
} void(0)


void KDE4AddUnitTest::go()
{
    ICore* core = ICore::self();
    IProjectController* projCtrl = core->projectController();

    projCtrl->openProject(m_projectFile);
    WAIT_FOR_OPEN_SIGNAL;
    QCOMPARE(1, projCtrl->projects().count());
    IProject* proj = projCtrl->projects()[0];

    // CMakeManager should have parsed the kde4_add_unit test 
    // - the project model should contain an exectuable named 'foo'
    // - projectfilemanager shoudl expose test info 'foo'

    ProjectFolderItem* root = proj->projectItem();
    QVERIFY2(root, "Project root item null");
    QVERIFY(root->folderList().isEmpty());
    QCOMPARE(1, root->targetList().count()); // the foo-executable target
    //QCOMPARE(3, root->fileList().count());   // foo.cpp, CMakeLists.txt & proj.kdev4

    ProjectExecutableTargetItem* fooTarget = root->targetList()[0]->executable();
    QVERIFY2(fooTarget, "Target is not an exectuable.");
    QCOMPARE(QVariant(QString("foo")), fooTarget->data(Qt::DisplayRole));

    IProjectFileManager* pfm = proj->projectFileManager();
    QList<Veritas::TestExecutableInfo> testInfo = pfm->testExecutables();
    QCOMPARE(1, testInfo.size());
    Veritas::TestExecutableInfo& test = testInfo[0];
    QCOMPARE(QString("foo"), test.name());
}

QTEST_KDEMAIN( KDE4AddUnitTest, GUI )
#include "kde4addunittest.moc"
