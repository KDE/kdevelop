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

#include "modelbuilder.h"
#include "suitebuilder.h"
#include <veritas/test.h>
#include "qtestsettings.h"
#include <interfaces/iproject.h>
#include <project/interfaces/ibuildsystemmanager.h>
#include <project/projectmodel.h>
#include <interfaces/iplugin.h>
#include <interfaces/icore.h>
#include <interfaces/iplugincontroller.h>
#include <interfaces/iuicontroller.h>
#include <veritas/ctestfileparser.h>

#include <KDebug>
#include <QThread>
#include <QDir>
#include <QApplication>
#include <KUrl>
#include <KLocale>
#include <KConfigGroup>

using QTest::ModelBuilder;
using QTest::SuiteBuilder;
using QTest::Settings;
using QTest::ISettings;
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

void ModelBuilder::connectBuilderPlugin(IPlugin* plugin)
{
    kDebug() << plugin << plugin->extensions();
    if (plugin->extensions().contains( "org.kdevelop.IProjectBuilder" )) {
        kDebug() << "setup connection";
        this->disconnect(plugin);
        connect(plugin, SIGNAL(built(KDevelop::ProjectBaseItem*)), 
                   SLOT(doReload(KDevelop::ProjectBaseItem*)));
    }
}

void ModelBuilder::doReload(KDevelop::ProjectBaseItem* item)
{
    if (item->project() == project()) {
        reload(project());
    }
}

/*! Read test executables from project configuration. Returns the empty list if not succesful */
QList<Veritas::TestExecutableInfo> fetchTestExesFromConfig(const KConfigGroup& veriConf, QString& failureMsg)
{
    QList<Veritas::TestExecutableInfo> testExes;
    if (!veriConf.exists()) {
        failureMsg = i18n("Failed to load test suite. No test executables configured.");
        return testExes;
    }
    if (veriConf.readEntry( "framework" ) != "QTest") {
        failureMsg = i18n("Failed to load test suite. Wrong framework.");
        return testExes;            
    }
    QStringList exes = veriConf.readEntry<QStringList>( "executables", QStringList() );
    foreach(const QString& exe, exes) {
            TestExecutableInfo te;
            KUrl testUrl(exe);
            if (!testUrl.isValid()) continue;
            te.setCommand(exe);
            te.setName(testUrl.fileName());
            te.setWorkingDirectory(testUrl.upUrl());
            testExes << te;
    }
    if (testExes.isEmpty()) {
        failureMsg = i18n("Failed to load test suite. No test executables configured.");
    }
    return testExes;
}

void ModelBuilder::reload(KDevelop::IProject* project_)
{
    if (!project_) return;
    if (m_reloading) return;
    m_reloading = true;    
    m_currentProject = project_;
    Q_ASSERT(!m_runner->isRunning());
    
    if (m_settings) delete m_settings;
    m_settings = new Settings(project());

    KConfigGroup proj = project()->projectConfiguration()->group("Project");
    if (proj.readEntry("Manager") == "KDevCMakeManager") {
        FilesystemAccess* fsm = new FilesystemAccess;
        CTestfileParser parser(fsm);
        parser.parse(buildRoot());
        m_testExes = parser.testExecutables();
        KConfigGroup veriConf = project()->projectConfiguration()->group( "Veritas" );
        veriConf.writeEntry( "framework", "QTest" );
        QStringList exes;
        foreach(const TestExecutableInfo& te, m_testExes) {
            exes << te.workingDirectory().resolved(te.command()).toLocalFile();
        }
        veriConf.writeEntry( "executables", exes);
        delete fsm;
    } else {
        QString failureMsg;
        KConfigGroup testConfig = project()->projectConfiguration()->group( "Veritas" );
        QList<Veritas::TestExecutableInfo> testExes;
        
        testExes = fetchTestExesFromConfig(testConfig, failureMsg);
        if (testExes.isEmpty()) {
            m_reloading = false;
            emit reloadFailed();
            emit showErrorMessage(failureMsg, 5);
            return;
        }
        m_testExes = testExes;
    }

    if (m_reloading) { 
        fetchTestCommands();
    }
}

ModelBuilder::ModelBuilder()
    : m_root(0),
      m_runner(new SuiteBuilderRunner),
      m_reloading(false),
      m_settings(0)
{
    foreach(IPlugin* i, ICore::self()->pluginController()->allPluginsForExtension( "org.kdevelop.IProjectBuilder" )) {
        connectBuilderPlugin(i);
    }
    connect(ICore::self()->pluginController(), 
            SIGNAL(pluginLoaded(KDevelop::IPlugin*)), 
            SLOT(connectBuilderPlugin(KDevelop::IPlugin*)));
    IUiController* uic = ICore::self()->uiController();
    uic->registerStatus(this);
    
    connect(m_runner, SIGNAL(finished()),
            SLOT(suiteBuilderFinished()));
}

ModelBuilder::~ModelBuilder()
{
    if (m_runner->m_suiteBuilder) delete m_runner->m_suiteBuilder;
    delete m_runner;
    if (m_settings) delete m_settings;
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

QString ModelBuilder::statusName() const
{
    return i18n("xTest");
}

typedef QMap<KUrl, ProjectExecutableTargetItem*> ExeTargetMap;

ProjectExecutableTargetItem* findTargetFor(const TestExecutableInfo& test, const ExeTargetMap& exeTargets)
{
    //If test.command() isn't relative, test.command() is used. Otherwise, the 
    //merged path with the working directory is used.
    KUrl testCmd(test.workingDirectory().resolved(test.command()));
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

KUrl ModelBuilder::buildRoot()
{
    IBuildSystemManager* bm = project()->buildSystemManager();
    STOP_IF(!bm, "Build system manager zero");

    KUrl buildRoot = bm->buildDirectory(project()->projectItem());
    STOP_IF(buildRoot.isEmpty(), "Root build directory empty");
    STOP_IF(buildRoot == KUrl("/./"), "Root build directory empty");
    
    return buildRoot;
}

void ModelBuilder::fetchTestCommands()
{
    Q_ASSERT(project());
    Q_ASSERT(m_reloading);

    KUrl buildRoot_ = buildRoot();
    if (!buildRoot_.isValid()) return;
    QDir buildDir(buildRoot_.toLocalFile());
    QList<Veritas::TestExecutableInfo> testExes;

    KConfigGroup proj = project()->projectConfiguration()->group("Project");
    if (proj.readEntry("Manager") == "KDevCMakeManager") {
    QMap<KUrl, ProjectExecutableTargetItem*> exeTargets;
    foreach(ProjectExecutableTargetItem* exe, fetchAllExeTargets(project()->projectItem())) {
        exeTargets[exe->builtUrl()] = exe;
    }
    foreach(TestExecutableInfo testInfo, m_testExes) {
        if (!KUrl(testInfo.command()).isValid()) continue;
        if (ProjectExecutableTargetItem* target = findTargetFor(testInfo, exeTargets)) {
            KUrl::List files;
            foreach(KDevelop::ProjectFileItem* f, target->fileList()) {
                files << f->url();
            }
            if (files.size() == 1) {
                testInfo.setSource( files[0] );
            }
            testInfo.setCommand( target->builtUrl().fileName() );
            testInfo.setWorkingDirectory( target->builtUrl().upUrl() );
            testExes << testInfo;
        }
    }} else {
        testExes = m_testExes;
    }

    if (m_runner->m_suiteBuilder) delete m_runner->m_suiteBuilder;
    SuiteBuilder* sb = new SuiteBuilder;
    sb->setTestExecutables(testExes);
    sb->setSettings(m_settings);

    m_runner->m_suiteBuilder = sb;
    connect(sb, SIGNAL(progress(int,int,int)),
            SLOT(slotShowProgress(int,int,int)), Qt::QueuedConnection);
    m_runner->start();
}

KDevelop::IProject* ModelBuilder::project() const
{
    return m_currentProject;
}

void ModelBuilder::slotShowProgress(int minimum, int maximum, int value)
{
    emit showProgress(this, minimum,maximum,value);
}

void ModelBuilder::suiteBuilderFinished()
{
    Q_ASSERT(m_runner);
    Q_ASSERT(m_reloading);
    Q_ASSERT(!m_runner->isRunning());
    Q_ASSERT(m_runner->m_suiteBuilder);
    m_root = m_runner->m_suiteBuilder->root();
    emit reloadFinished(m_root);
    emit hideProgress(this);
    m_reloading = false;
}

Test* ModelBuilder::root() const
{
    Q_ASSERT(m_root);
    return m_root;
}

#include "modelbuilder.moc"
