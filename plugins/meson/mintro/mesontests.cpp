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

#include <utility>

using namespace std;
using namespace KDevelop;

// Class MesonTest

MesonTest::MesonTest(const QJsonObject& json, IProject* project)
    : m_project(project)
{
    fromJson(json);
}

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

    auto* job = new OutputExecuteJob(m_project, convVerbosity);
    *job << m_command;
    if (!m_workDir.isEmpty()) {
        job->setWorkingDirectory(m_workDir.toUrl());
    }
    job->setJobName(m_name);
    for (auto i = cbegin(m_env); i != cend(m_env); ++i) {
        job->addEnvironmentOverride(i.key(), i.value());
    }
    return job;
}

void MesonTest::fromJson(const QJsonObject& json)
{
    m_name = json[QStringLiteral("name")].toString();
    m_workDir = Path(json[QStringLiteral("workdir")].toString());

    const QJsonArray cmd = json[QStringLiteral("cmd")].toArray();
    const QJsonArray suites = json[QStringLiteral("suite")].toArray();
    const QJsonObject env = json[QStringLiteral("env")].toObject();

    m_command.reserve(cmd.size());
    transform(begin(cmd), end(cmd), back_inserter(m_command), [](const auto& x) { return x.toString(); });
    m_suites.reserve(suites.size());
    transform(begin(suites), end(suites), back_inserter(m_suites), [](const auto& x) { return x.toString(); });

    for (auto i = begin(env); i != end(env); ++i) {
        m_env[i.key()] = i.value().toString();
    }

    qCDebug(KDEV_Meson) << "MINTRO:   - Loaded test" << m_name << "suites:" << m_suites;
}

// Class MesonTestSuite

MesonTestSuite::MesonTestSuite(QString name, IProject* project)
    : m_name(std::move(name))
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
    result.reserve(m_tests.size());
    for (auto& i : m_tests) {
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
    const auto iter = m_tests.constFind(testCase);
    if (iter == cend(m_tests)) {
        return nullptr;
    }

    return (*iter)->job(verbosity);
}

KJob* MesonTestSuite::launchCases(const QStringList& testCases, TestJobVerbosity verbosity)
{
    QList<KJob*> jobs;
    jobs.reserve(testCases.size());
    for (const auto& i : testCases) {
        if (auto* job = launchCase(i, verbosity)) {
            jobs << job;
        }
    }

    return new ExecuteCompositeJob(m_project, jobs);
}

KJob* MesonTestSuite::launchAllCases(TestJobVerbosity verbosity)
{
    QList<KJob*> jobs;
    jobs.reserve(m_tests.size());
    for (auto& i : qAsConst(m_tests)) {
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

    m_tests[test->name()] = std::move(test);
}

MesonTestSuites mesonTestSuitesFromJson(const QJsonArray& json, IProject* project)
{
    qCDebug(KDEV_Meson) << "MINTRO: -- Loading tests from JSON...";
    MesonTestSuites testSuites;
    for (const auto& jsonTest : json) {
        const auto test = make_shared<MesonTest>(jsonTest.toObject(), project);
        for (const QString& suiteName : test->suites()) {
            auto& suite = testSuites[suiteName];
            if (!suite) {
                suite = new MesonTestSuite(suiteName, project);
            }
            suite->addTestCase(test);
            qCDebug(KDEV_Meson) << "MINTRO:   - Added test" << test->name() << "to suite" << suite->name();
        }
    }
    return testSuites;
}
