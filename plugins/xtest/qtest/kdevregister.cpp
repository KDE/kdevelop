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
#include "suitebuilder.h"
#include <veritas/test.h>
#include "qtestsettings.h"
#include <interfaces/iproject.h>
#include <project/interfaces/ibuildsystemmanager.h>
#include <project/projectmodel.h>
#include <interfaces/iuicontroller.h>
#include <veritas/ctestfileparser.h>
#include <interfaces/icore.h>
#include <KDebug>
#include <QThread>
#include <QDir>
#include <QApplication>
#include <KUrl>
#include <KLocale>
#include <KConfigGroup>

using QTest::KDevRegister;
using QTest::SuiteBuilder;
using QTest::Settings;
using Veritas::Test;
using Veritas::TestExecutableInfo;
using Veritas::CTestfileParser;
using Veritas::FilesystemAccess;
using namespace KDevelop;

namespace
{

/*! Recurses down the project item tree and collects all ExecutableTargets. */
QList<ProjectExecutableTargetItem*> fetchAllExeTargets(ProjectBaseItem* root)
{
    QList<ProjectExecutableTargetItem*> exes;
    if (!root) return exes;
    foreach(ProjectTargetItem* exe, root->targetList()) {
        if (ProjectExecutableTargetItem* exe_ = exe->executable()) {
            exes << exe_;
        }
    }
    foreach(ProjectFolderItem* fldr, root->folderList()) {
        exes += fetchAllExeTargets(fldr);
    }
    return exes;
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
    return KUrl(); \
} else void(0)

void KDevRegister::reload()
{
    Q_ASSERT(project());
    Q_ASSERT(m_runner);
    if (m_reloading) return;
    Q_ASSERT(!m_runner->isRunning());

    m_reloading = true;

    KConfigGroup proj = project()->projectConfiguration()->group("Project");
    if (proj.readEntry("Manager") == "KDevCMakeManager") {
        FilesystemAccess* fsm = new FilesystemAccess;
        CTestfileParser parser(fsm);
        parser.parse(buildRoot());
        m_testExes = parser.testExecutables();
        delete fsm;
    } else { // TODO
        emit reloadFailed();
        emit showErrorMessage(i18n("Failed to fetch test locations. Not a CMake project [todo]"), 5);
        return;
    }

    fetchTestCommands(0);
}

QString KDevRegister::statusName() const
{
    return i18n("xTest");
}

typedef QMap<KUrl, ProjectExecutableTargetItem*> ExeTargetMap;

ProjectExecutableTargetItem* findTargetFor(const TestExecutableInfo& test, const ExeTargetMap& exeTargets)
{
    KUrl testCmd(test.command());
    QString testFile = testCmd.fileName();
    ProjectExecutableTargetItem* exe = 0;
    if (testFile.endsWith(".shell")) {
        testFile.chop(6);
        testCmd = KUrl(testCmd.upUrl(), testFile);
        if (exeTargets.contains(testCmd)) {
            exe = exeTargets[testCmd];
        }
    } else if (exeTargets.contains(testCmd)) {
        exe = exeTargets[testCmd];
    }
    return exe;
}

KUrl KDevRegister::buildRoot()
{
    IBuildSystemManager* bm = project()->buildSystemManager();
    STOP_IF(!bm, "Build system manager zero");

    KUrl buildRoot = bm->buildDirectory(project()->projectItem());
    STOP_IF(buildRoot.isEmpty(), "Root build directory empty");
    STOP_IF(buildRoot == KUrl("/./"), "Root build directory empty");
    
    return buildRoot;
}

void KDevRegister::fetchTestCommands(KJob*)
{
    kDebug() << "";
    Q_ASSERT(project());
    Q_ASSERT(m_reloading);

    KUrl buildRoot_ = buildRoot();
    if (!buildRoot_.isValid()) return;
    QDir buildDir(buildRoot_.path());
    KUrl::List testExes;

    QMap<KUrl, ProjectExecutableTargetItem*> exeTargets;
    foreach(ProjectExecutableTargetItem* exe, fetchAllExeTargets(project()->projectItem())) {
        exeTargets[exe->builtUrl()] = exe;
    }
    foreach(const TestExecutableInfo& testInfo, m_testExes) {
        if (!KUrl(testInfo.command()).isValid()) continue;
        if (ProjectExecutableTargetItem* target = findTargetFor(testInfo, exeTargets)) {
            testExes << target->builtUrl();
        }
    }

    if (m_runner->m_suiteBuilder) delete m_runner->m_suiteBuilder;
    SuiteBuilder* sb = new SuiteBuilder;
    sb->setTestExecutables(testExes);
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


