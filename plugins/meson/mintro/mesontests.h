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

#pragma once

#include <QHash>
#include <QObject>
#include <QString>
#include <QStringList>
#include <interfaces/itestsuite.h>
#include <language/duchain/indexeddeclaration.h>
#include <memory>
#include <util/path.h>

class QJsonObject;
class QJsonArray;
class MesonTest;
class MesonTestSuite;
class MesonTestSuites;
using MesonTestPtr = std::shared_ptr<MesonTest>;
using MesonTestSuitePtr = std::shared_ptr<MesonTestSuite>;
using MesonTestSuitesPtr = std::shared_ptr<MesonTestSuites>;

class MesonTest
{
public:
    explicit MesonTest(QJsonObject const& json, KDevelop::IProject* project);
    virtual ~MesonTest();

    QString name() const;
    QStringList suites() const;
    KDevelop::IProject* project() const;
    KJob* job(KDevelop::ITestSuite::TestJobVerbosity verbosity);

    void fromJson(QJsonObject const& json);

private:
    QString m_name;
    QStringList m_command;
    QStringList m_suites; // In meson one test can be part of multiple suites
    KDevelop::Path m_workDir;
    QHash<QString, QString> m_env;
    KDevelop::IProject* m_project = nullptr;
};

class MesonTestSuite : public KDevelop::ITestSuite
{
public:
    explicit MesonTestSuite(QString name, KDevelop::IProject* project);
    virtual ~MesonTestSuite();

    // Implementing the ITestSuite interface
    QString name() const override;
    QStringList cases() const override;
    KDevelop::IProject* project() const override;
    KJob* launchCase(const QString& testCase, KDevelop::ITestSuite::TestJobVerbosity verbosity) override;
    KJob* launchCases(const QStringList& testCases, KDevelop::ITestSuite::TestJobVerbosity verbosity) override;
    KJob* launchAllCases(KDevelop::ITestSuite::TestJobVerbosity verbosity) override;

    // TODO implement these
    KDevelop::IndexedDeclaration declaration() const override;
    KDevelop::IndexedDeclaration caseDeclaration(const QString& testCase) const override;

    // Custom functions
    void addTestCase(MesonTestPtr test);
    QHash<QString, MesonTestPtr> tests();

private:
    QString m_name;
    KDevelop::IProject* m_project = nullptr;
    QHash<QString, MesonTestPtr> m_tests;
};

class MesonTestSuites
{
public:
    explicit MesonTestSuites(QJsonArray const& json, KDevelop::IProject* project);
    virtual ~MesonTestSuites();

    QHash<QString, MesonTestSuitePtr> testSuites();
    MesonTestSuitePtr testSuite(QString name);
    MesonTestSuitePtr operator[](QString name);

    void fromJSON(QJsonArray const& json);

private:
    KDevelop::IProject* m_project = nullptr;
    QHash<QString, MesonTestSuitePtr> m_suites;
};
