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
#include <project/interfaces/ibuildsystemmanager.h>
#include <project/projectmodel.h>
#include <interfaces/iuicontroller.h>

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
    foreach(QFileInfo fi, files) {
        if (testNames.contains(fi.fileName())) {
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
    IUiController* uic = ICore::self()->uiController();
    uic->registerStatus(this);
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
    emit showErrorMessage(MSG, 5); \
    return; \
} else void(0)

void KDevRegister::reload()
{
    Q_ASSERT(project());
    Q_ASSERT(m_runner);
    if (m_reloading) return;
    Q_ASSERT(!m_runner->isRunning());

    m_reloading = true;
    QList<KDevelop::ProjectTestTargetItem*> testTargets = fetchAllTestTargets(project()->projectItem());
    m_testNames = namesFromTargets(testTargets);
    fetchTestCommands(0);
}

QString KDevRegister::statusName() const
{
    return i18n("xTest");
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
            SLOT(suiteBuilderFinished()));
    connect(m_runner, SIGNAL(finished()),
            SIGNAL(hideProgress()));
    connect(sb, SIGNAL(progress(int,int,int)),
            SIGNAL(showProgress(int,int,int)), Qt::QueuedConnection);

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
    emit reloadFinished(m_root);
    m_reloading = false;
}

Test* KDevRegister::root() const
{
    Q_ASSERT(m_root);
    return m_root;
}


