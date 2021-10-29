/*
    SPDX-FileCopyrightText: 2019 Daniel Mensinger <daniel@mensinger-ka.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
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
    explicit MesonTest(const QJsonObject& json, KDevelop::IProject* project);
    virtual ~MesonTest();

    QString name() const;
    QStringList suites() const;
    KDevelop::IProject* project() const;
    KJob* job(KDevelop::ITestSuite::TestJobVerbosity verbosity);

    void fromJson(const QJsonObject& json);

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
    explicit MesonTestSuites(const QJsonArray& json, KDevelop::IProject* project);
    virtual ~MesonTestSuites();

    QHash<QString, MesonTestSuitePtr> testSuites();
    MesonTestSuitePtr testSuite(QString name);
    MesonTestSuitePtr operator[](QString name);

    void fromJSON(const QJsonArray& json);

private:
    KDevelop::IProject* m_project = nullptr;
    QHash<QString, MesonTestSuitePtr> m_suites;
};
