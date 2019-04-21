/* This file is part of KDevelop
    Copyright 2019 Daniel Mensinger <daniel@mensinger-ka.de>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "mesontests.h"
#include <QJsonArray>
#include <QJsonObject>
#include <algorithm>
#include <debug.h>
#include <interfaces/iproject.h>
#include <kjob.h>
#include <outputview/outputexecutejob.h>
#include <util/executecompositejob.h>

using namespace std;
using namespace KDevelop;

// Class MesonTest

MesonTest::MesonTest(QJsonObject const& json, IProject* project)
    : m_project(project)
{
    fromJson(json);
}

MesonTest::~MesonTest() {}

QString MesonTest::name() const
{
    return m_name;
}

QStringList MesonTest::suites() const
{
    return m_suites;
}

IProject* MesonTest::project() const
{
    return m_project;
}

KJob* MesonTest::job(ITestSuite::TestJobVerbosity verbosity)
{
    OutputJob::OutputJobVerbosity convVerbosity;
    switch (verbosity) {
    case KDevelop::ITestSuite::Verbose:
        convVerbosity = OutputJob::Verbose;
        break;
    case KDevelop::ITestSuite::Silent:
        convVerbosity = OutputJob::Silent;
        break;
    }

    OutputExecuteJob* job = new OutputExecuteJob(m_project, convVerbosity);
    *job << m_command;
    if (!m_workDir.isEmpty()) {
        job->setWorkingDirectory(m_workDir.toUrl());
    }
    job->setJobName(m_name);
    for (auto i = begin(m_env); i != end(m_env); ++i) {
        job->addEnvironmentOverride(i.key(), i.value());
    }
    return job;
}

void MesonTest::fromJson(const QJsonObject& json)
{
    m_name = json[QStringLiteral("name")].toString();
    m_workDir = Path(json[QStringLiteral("workdir")].toString());

    QJsonArray cmd = json[QStringLiteral("cmd")].toArray();
    QJsonArray suites = json[QStringLiteral("suite")].toArray();
    QJsonObject env = json[QStringLiteral("env")].toObject();

    transform(begin(cmd), end(cmd), back_inserter(m_command), [](auto const& x) { return x.toString(); });
    transform(begin(suites), end(suites), back_inserter(m_suites), [](auto const& x) { return x.toString(); });

    for (auto i = begin(env); i != end(env); ++i) {
        m_env[i.key()] = i.value().toString();
    }

    qCDebug(KDEV_Meson) << "MINTRO:   - Loaded test" << m_name << "suites:" << m_suites;
}

// Class MesonTestSuite

MesonTestSuite::MesonTestSuite(QString name, IProject* project)
    : m_name(name)
    , m_project(project)
{
    qCDebug(KDEV_Meson) << "MINTRO:   - New test suite" << m_name;
}

MesonTestSuite::~MesonTestSuite() {}

QString MesonTestSuite::name() const
{
    return m_name;
}

QStringList MesonTestSuite::cases() const
{
    QStringList result;
    for (auto i : m_tests) {
        result << i->name();
    }
    return result;
}

IProject* MesonTestSuite::project() const
{
    return m_project;
}

KJob* MesonTestSuite::launchCase(const QString& testCase, TestJobVerbosity verbosity)
{
    auto iter = m_tests.find(testCase);
    if (iter == end(m_tests)) {
        return nullptr;
    }

    return (*iter)->job(verbosity);
}

KJob* MesonTestSuite::launchCases(const QStringList& testCases, TestJobVerbosity verbosity)
{
    QList<KJob*> jobs;
    for (auto const& i : testCases) {
        auto iter = m_tests.find(i);
        if (iter == end(m_tests)) {
            continue;
        }

        jobs << (*iter)->job(verbosity);
    }

    return new ExecuteCompositeJob(m_project, jobs);
}

KJob* MesonTestSuite::launchAllCases(TestJobVerbosity verbosity)
{
    QList<KJob*> jobs;
    for (auto& i : m_tests) {
        jobs << i->job(verbosity);
    }

    return new ExecuteCompositeJob(m_project, jobs);
}

IndexedDeclaration MesonTestSuite::declaration() const
{
    return IndexedDeclaration();
}

IndexedDeclaration MesonTestSuite::caseDeclaration(const QString&) const
{
    return IndexedDeclaration();
}

void MesonTestSuite::addTestCase(MesonTestPtr test)
{
    if (!test) {
        qCDebug(KDEV_Meson) << "TESTS: Tried to add a nullptr test";
        return;
    }

    m_tests[test->name()] = test;
}

QHash<QString, MesonTestPtr> MesonTestSuite::tests()
{
    return m_tests;
}

// Class MesonTestSuites

MesonTestSuites::MesonTestSuites(const QJsonArray& json, IProject* project)
    : m_project(project)
{
    fromJSON(json);
}

MesonTestSuites::~MesonTestSuites() {}

QHash<QString, MesonTestSuitePtr> MesonTestSuites::testSuites()
{
    return m_suites;
}

MesonTestSuitePtr MesonTestSuites::testSuite(QString name)
{
    auto iter = m_suites.find(name);
    if (iter == end(m_suites)) {
        return nullptr;
    }
    return *iter;
}

MesonTestSuitePtr MesonTestSuites::operator[](QString name)
{
    return testSuite(name);
}

void MesonTestSuites::fromJSON(const QJsonArray& json)
{
    QVector<MesonTestPtr> tests;
    qCDebug(KDEV_Meson) << "MINTRO: -- Loading tests from JSON...";
    for (auto const& i : json) {
        tests << make_shared<MesonTest>(i.toObject(), m_project);
    }

    qCDebug(KDEV_Meson) << "MINTRO: -- Adding tests to suites";
    for (auto& i : tests) {
        for (QString j : i->suites()) {
            auto suite = testSuite(j);
            if (!suite) {
                suite = make_shared<MesonTestSuite>(j, m_project);
                m_suites[j] = suite;
            }
            suite->addTestCase(i);
            qCDebug(KDEV_Meson) << "MINTRO:   - Added test" << i->name() << "to suite" << suite->name();
        }
    }
}
