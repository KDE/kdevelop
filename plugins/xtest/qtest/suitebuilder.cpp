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
#include "config/qtestsettings.h"
#include <veritas/test.h>
#include <KProcess>
#include <KUrl>
#include <KLocale>
#include <KDebug>
#include <QStringList>

using QTest::SuiteBuilder;
using QTest::QTestCase;
using QTest::QTestOutputParser;
using QTest::QTestSuite;
using QTest::Executable;
using QTest::CaseBuilder;

using Veritas::Test;

SuiteBuilder::SuiteBuilder()
    : m_root(0), m_hasRun(false), m_testExesSet(false), m_settings(0)
{
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

void SuiteBuilder::addSuiteName(const KUrl& exeUrl)
{
    KUrl suiteUrl = exeUrl.upUrl();
    if (m_suiteNames.contains(suiteUrl)) return;
    QMapIterator<KUrl, QString> it(m_suiteNames);
    QString suiteName = suiteNameForExe(exeUrl);
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
    foreach(const KUrl& testExe, m_testShellExes) {
        addSuiteName(testExe);
    }
    foreach(const KUrl& testExe, m_testShellExes) {
        QString suiteName = m_suiteNames[testExe.upUrl()];
        if (!m_suites.contains(suiteName)) {
            QFileInfo suiteDir(testExe.upUrl().path());
            QTestSuite* suite = new QTestSuite(suiteName, suiteDir, m_root);
            m_root->addChild(suite);
            m_suites[suiteName] = suite;
        }
    }
}

void SuiteBuilder::constructCases()
{
    int nrofShells = m_testShellExes.count();
    int count = 1;
    foreach(const KUrl& testExe, m_testShellExes) {
        QString suiteName = m_suiteNames[testExe.upUrl()];
        CaseBuilder* cb = createCaseBuilder(testExe);
        cb->setSuiteName(suiteName);
        QTestCase* caze = cb->construct();
        delete cb;

        Q_ASSERT(m_suites.contains(suiteName));
        QTestSuite* suite = m_suites[suiteName];
        Q_ASSERT(suite);
        suite->addChild(caze);
        caze->setParent(suite);
        caze->setProcess(new KProcess);
        caze->setOutputParser(new QTestOutputParser);
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

void SuiteBuilder::setTestExecutables(const QList<KUrl>& testExes)
{
    Q_ASSERT(!hasRun()); Q_ASSERT(!m_testExesSet);
    m_testShellExes = testExes;
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
