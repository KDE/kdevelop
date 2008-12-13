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

#include "suitebuilder.h"
#include "casebuilder.h"
#include "qtestcase.h"
#include "qtestoutputparser.h"
#include "qtestsuite.h"
#include "executable.h"
#include <interfaces/iplugin.h>
#include "qtestsettings.h"
#include <veritas/test.h>
#include <veritas/testexecutableinfo.h>
#include <KProcess>
#include <KUrl>
#include <KLocale>
#include <KDebug>
#include <QStringList>

using QTest::SuiteBuilder;
using QTest::Case;
using QTest::OutputParser;
using QTest::Suite;
using QTest::Executable;
using QTest::CaseBuilder;
using QTest::ISettings;

using Veritas::Test;
using Veritas::TestExecutableInfo;

SuiteBuilder::SuiteBuilder()
    : m_root(0), m_hasRun(false), m_testExesSet(false), m_settings(0), m_previousRoot(0)
{
}

void SuiteBuilder::setPreviousRoot(Test* previousRoot)
{
    Q_ASSERT(m_previousRoot == 0); Q_ASSERT(previousRoot != 0);
    m_previousRoot = previousRoot;
}

void SuiteBuilder::initRoot()
{
    Q_ASSERT(m_root == 0);
    m_root = Test::createRoot();
}

namespace {
QStringList testDirs(QString("tests,test,Tests").split(','));
}

/*! deduce the suite name for a test exe name. take the directory name
or 2 dirs up if it is a canonical name such as tests, test etc. */
QString SuiteBuilder::suiteNameForExe(const KUrl& testExe)
{
    KUrl dir = testExe.upUrl();
    if (testDirs.contains(dir.fileName())) {
        dir = dir.upUrl();
    }
    return dir.fileName();
}

QString SuiteBuilder::suiteNameOneUp(const KUrl& suiteUrl)
{
    KUrl dir = suiteUrl;
    if (testDirs.contains(dir.fileName())) {
        dir = dir.upUrl();
    }
    KUrl up = dir.upUrl();
    return up.fileName() + '-' + dir.fileName();
}

void SuiteBuilder::addSuiteName(const TestExecutableInfo& exe)
{
    KUrl suiteUrl = exe.workingDirectory();
    if (m_suiteNames.contains(suiteUrl)) return;
    QMapIterator<KUrl, QString> it(m_suiteNames);
    QString suiteName = suiteNameForExe(KUrl(suiteUrl, exe.command()));
    KUrl collision;
    while(it.hasNext()) {
        it.next();
        if (it.value() == suiteName) { // name collision
            suiteName = suiteNameOneUp(suiteUrl);
            collision = it.key();
            break;
        }
    }
    if (!collision.isEmpty()) {
        m_suiteNames[collision] = suiteNameOneUp(collision);
    }
    m_suiteNames[suiteUrl] = suiteName;
}

void SuiteBuilder::constructSuites()
{
    // create a suite per test directory.
    Q_ASSERT(m_root); Q_ASSERT(m_testExesSet);
    foreach(const TestExecutableInfo& testExe, m_testExes) {
        addSuiteName(testExe);
    }
    foreach(const TestExecutableInfo& testExe, m_testExes) {
        QString suiteName = m_suiteNames[testExe.workingDirectory()];
        if (!m_suites.contains(suiteName)) {
            QFileInfo suiteDir(testExe.workingDirectory().path());
            Suite* suite = new Suite(suiteName, suiteDir, m_root);
            m_suites[suiteName] = suite;
        }
    }
    QMap<QString, Suite*>::Iterator it;
    for(it = m_suites.begin(); it != m_suites.end(); it++) {
        m_root->addChild(it.value());
    }
}

void SuiteBuilder::constructCases()
{
    int nrofShells = m_testExes.count();
    int count = 1;
    foreach(const TestExecutableInfo& testExeInfo, m_testExes) {
        KUrl testExe = KUrl(testExeInfo.workingDirectory(), testExeInfo.command());
        kDebug() << testExe;
        QString suiteName = m_suiteNames[testExe.upUrl()];
        CaseBuilder* cb = createCaseBuilder(testExe);
        cb->setSuiteName(suiteName);
        Case* caze = cb->construct();
        delete cb;
        if (!caze) continue;
        if (testExeInfo.source().isValid()) {
            caze->setSupportsToSource( true );
            caze->setSource( testExeInfo.source() );
            for(int i=0; i<caze->childCount(); i++) {
                Command* cmd = caze->child(i);
                cmd->setSupportsToSource( true );
            }
        }
        Q_ASSERT(m_suites.contains(suiteName));
        Suite* suite = m_suites[suiteName];
        Q_ASSERT(suite);
        suite->addChild(caze);
        caze->setParent(suite);
        caze->setProcess(new KProcess);
        caze->setOutputParser(new OutputParser);
        caze->setSettings(m_settings);
        emit progress(0, nrofShells, count);
        count++;
    }
}

void SuiteBuilder::start()
{
    Q_ASSERT_X(!hasRun(), "SuiteBuilder::start()", "Throw-away object, do not reuse.");
    Q_ASSERT(m_testExesSet);
    initRoot();
    constructSuites();
    constructCases();
    m_hasRun = true;
}

SuiteBuilder::~SuiteBuilder()
{
}

void SuiteBuilder::setSettings(ISettings* settings)
{
    m_settings = settings;
}

void SuiteBuilder::setTestExecutables(const QList<Veritas::TestExecutableInfo>& testExes)
{
    Q_ASSERT(!hasRun()); Q_ASSERT(!m_testExesSet);
    m_testExes = testExes;
    m_testExesSet = true;
}

Veritas::Test* SuiteBuilder::root() const
{
    Q_ASSERT(hasRun());
    return m_root;
}

CaseBuilder* SuiteBuilder::createCaseBuilder(const KUrl& testLocation) const
{
    Q_ASSERT(!hasRun());
    CaseBuilder* cb = new CaseBuilder();
    Executable* exe = new Executable();
    exe->setLocation(testLocation);
    cb->setExecutable(exe);
    return cb;
}

bool SuiteBuilder::hasRun() const
{
    return m_hasRun;
}

#include "suitebuilder.moc"
