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
#include <project/interfaces/iprojectbuilder.h>
#include "documentaccess.h"
#include "suitebuilder.h"
#include <veritas/test.h>
#include "config/qtestsettings.h"
#include <interfaces/iproject.h>
#include <project/projectmodel.h>
#include <project/interfaces/ibuildsystemmanager.h>
#include <interfaces/iruncontroller.h>
#include <interfaces/irun.h>
#include <interfaces/iplugin.h>

#include <interfaces/icore.h>
#include <KDebug>
#include <QThread>
#include <QDir>
#include <QApplication>
#include <KUrl>
#include <KLocale>

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

bool isShellFile(const QFileInfo& file)
{
    return file.suffix() == "shell";
}

/*! Remove the corresponding bare executable for each '.shell' */
void favorShellOverBareExe(QFileInfoList& files)
{
    QStringList shellBaseNames;
    foreach(QFileInfo fi, files) {
        if (isShellFile(fi)) {
            shellBaseNames << fi.baseName();
        }
    }
    QFileInfoList duplicates;
    foreach(QFileInfo fi, files) {
        if (!isShellFile(fi)) {
            if (shellBaseNames.contains(fi.baseName())) {
                duplicates << fi;
            }
        }
    }
    foreach(QFileInfo dupli, duplicates) {
        files.removeOne(dupli);
    }
}

/*! Locate all test executables in @p dir for which the name is contained in
@p testNames. This function recurses down @p dir and its subdirectories.
Return the found test exes as a list of  QFileInfo's. */
QFileInfoList findTestExesIn(QDir& dir, const QStringList& testNames)
{
    kDebug() << dir.absolutePath();
    QFileInfoList testExes;
    QDir current(dir);
    current.setFilter(QDir::Dirs | QDir::NoDotAndDotDot | QDir::Readable | QDir::Writable);
    QStringList subDirs = current.entryList();
    foreach(const QString& subDir, subDirs) {
        if (subDir == "CMakeFiles") continue;
        current.cd(subDir);
        testExes += findTestExesIn(current, testNames);
        current.cdUp();
    }
    current = QDir(dir);
    current.setFilter(QDir::Files |  QDir::Writable | QDir::NoSymLinks | QDir::Executable);
    QFileInfoList files = current.entryInfoList();
    favorShellOverBareExe(files);
    foreach(QFileInfo fi, files) {
        if (testNames.contains(fi.baseName())) {
            testExes << fi;
        }
    }
    return testExes;
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

class SuiteBuilderRunner : public QThread
{
public:
    SuiteBuilderRunner() : m_suiteBuilder(0){}
    virtual ~SuiteBuilderRunner() {}
    void run() {
        Q_ASSERT(m_suiteBuilder);
        m_suiteBuilder->start();
        Test* root = m_suiteBuilder->root();
        root->moveToThread(QApplication::instance()->thread());
    }
    SuiteBuilder* m_suiteBuilder;
};


KDevRegister::KDevRegister()
    : m_root(0), m_runner(new SuiteBuilderRunner), m_reloading(false)
{
    m_runController = ICore::self()->runController();
}

KDevRegister::~KDevRegister()
{
    if (m_runner->m_suiteBuilder) delete m_runner->m_suiteBuilder;
    delete m_runner;
}

#define STOP_IF(X, MSG) \
if (X) {\
    m_root = new Test("");\
    m_reloading = false; \
    emit reloadFailed(); \
    kDebug() << MSG; \
    return; \
} else void(0)

void KDevRegister::reload()
{
    Q_ASSERT(project());
    Q_ASSERT(m_runController);
    Q_ASSERT(m_runner);
    if (m_reloading) return;
    Q_ASSERT(!m_runner->isRunning());

    registerStatus();

    m_reloading = true;
    m_testTargets = fetchAllTestTargets(project()->projectItem());
    m_testNames = namesFromTargets(m_testTargets);
    fetchTestCommands(0);

    emit showErrorMessage(QString("FOOBAR"), 10);
}

void KDevRegister::buildTests()
{
    kDebug() << "";
    Q_ASSERT(project());

    IBuildSystemManager* bm = project()->buildSystemManager();
    STOP_IF(!bm, "Build system manager zero");

//     This is way too slow with cmake's dependency.
//     IProjectBuilder* ipb = bm->builder(0);
//     foreach(ProjectTestTargetItem* test, m_testTargets) {
//         KJob* make = ipb->build(test);
//         bool succ = make->exec();
//         if (!succ) {
//             m_testNames.removeAll(test->data(Qt::DisplayRole).toString());
//         }
//     }

    KUrl buildRoot = bm->buildDirectory(project()->projectItem());
    STOP_IF(buildRoot.isEmpty(), "Root build directory empty");
    STOP_IF(buildRoot == KUrl("/./"), "Root build directory empty");

    // Still way too slow, a full rebuild is actually faster for a project
    // without modifications.
    IRun makeRun = KDevelop::ICore::self()->runController()->defaultRun();
    makeRun.setExecutable(settings()->makeBinary());
    makeRun.setArguments(m_testNames);
    makeRun.setWorkingDirectory(buildRoot.path());
    KJob* make = m_runController->execute(makeRun);
    connect(make, SIGNAL(finished(KJob*)), this, SLOT(fetchTestCommands(KJob*)));
}

QString KDevRegister::statusName() const
{
    return i18n("xTest");
}

#include <QMainWindow>
#include <QStatusBar>

#include <interfaces/iuicontroller.h>

void KDevRegister::registerStatus()
{
    // TODO get rid of this joke. Factually istatus should not be
    // limitted to plugins + backgroundparser, but exposed for all

//     QWidget* mw = QApplication::activeWindow();
//     if (!mw) { kDebug() << "No mw"; return; }
//     QList<QStatusBar*> sbs = mw->findChildren<QStatusBar*>();
//     if (sbs.isEmpty()) { kDebug() << "No statusbar"; return; }
//     QStatusBar* sb = sbs[0];
//     if (!sb) { kDebug() << "sb zero"; return; }
//     connect(this, SIGNAL(showProgress(int,int,int)),
//             sb, SLOT(showProgress(int,int,int)));
//     connect(this, SIGNAL(hideProgress()),
//             sb, SLOT(hideProgress()));
//     connect(this, SIGNAL(showErrorMessage(QString,int)),
//             sb, SLOT(showErrorMessage(QString,int)));
    IUiController* uic = ICore::self()->uiController();
    uic->registerStatus(this);
}


void KDevRegister::fetchTestCommands(KJob*)
{
    kDebug() << "";
    Q_ASSERT(project());
    Q_ASSERT(m_reloading);

    IBuildSystemManager* bm = project()->buildSystemManager();
    STOP_IF(!bm, "Build system manager zero");

    KUrl buildRoot = bm->buildDirectory(project()->projectItem());
    STOP_IF(buildRoot.isEmpty(), "Root build directory empty");
    STOP_IF(buildRoot == KUrl("/./"), "Root build directory empty");

    QDir buildDir(buildRoot.path());
    QFileInfoList shells = findTestExesIn(buildDir, m_testNames);
    foreach(QFileInfo shell, shells) {
        kDebug() << "shell -> " << shell.absoluteFilePath();
    }
    STOP_IF(shells.isEmpty(), "No test exes found.");

    QList<KUrl> shellUrls = fileInfo2KUrl(shells);
    if (m_runner->m_suiteBuilder) delete m_runner->m_suiteBuilder;
    SuiteBuilder* sb = new SuiteBuilder;
    sb->setTestExecutables(shellUrls);
    sb->setSettings(settings());

    m_runner->m_suiteBuilder = sb;
    connect(m_runner, SIGNAL(finished()),
            this, SLOT(suiteBuilderFinished()));
    connect(m_runner, SIGNAL(finished()),
            this, SIGNAL(hideProgress()));
    connect(sb, SIGNAL(progress(int,int,int)),
            this, SIGNAL(showProgress(int,int,int)), Qt::QueuedConnection);

    m_runner->start();
}

void KDevRegister::suiteBuilderFinished()
{
    kDebug() << "";
    Q_ASSERT(m_runner);
    Q_ASSERT(m_reloading);
    Q_ASSERT(!m_runner->isRunning());
    Q_ASSERT(m_runner->m_suiteBuilder);
    m_root = m_runner->m_suiteBuilder->root();
    kDebug() << m_root;
    emit reloadFinished(m_root);
    m_reloading = false;
}

Test* KDevRegister::root() const
{
    Q_ASSERT(m_root);
    return m_root;
}


