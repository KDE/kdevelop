/*
    SPDX-FileCopyrightText: 2014 Milian Wolff <mail@milianw.de>
    SPDX-FileCopyrightText: 2023 Igor Kushnir <igorkuo@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "test_executecompositejob.h"

#include <util/executecompositejob.h>

#include <QStandardPaths>
#include <QStringList>
#include <QTest>

#include <utility>

using namespace KDevelop;

class TestJob : public KJob
{
    void start() override
    {
    }
};

class TestCompositeJob : public ExecuteCompositeJob
{
public:
    using ExecuteCompositeJob::ExecuteCompositeJob;
    using ExecuteCompositeJob::subjobs;
};

void TestExecuteCompositeJob::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
}

void TestExecuteCompositeJob::create_data()
{
    QTest::addColumn<QStringList>("subjobNames");
    QTest::addColumn<QString>("expectedCompositeJobName");

    QTest::newRow("no subjobs") << QStringList{} << "";
    QTest::newRow("empty-named subjob") << QStringList{""} << "";
    QTest::newRow("named subjob") << QStringList{"run as"} << "run as";
    QTest::newRow("two empty-named subjobs") << QStringList{"", ""} << "";
    QTest::newRow("empty-named and named") << QStringList{"", "pick"} << "pick";
    QTest::newRow("named and empty-named") << QStringList{"take", ""} << "take";
    QTest::newRow("two named subjobs") << QStringList{"x", "y"} << "x";
    QTest::newRow("three empty-named subjobs") << QStringList{"", "", ""} << "";
    QTest::newRow("empty-named and two named") << QStringList{"", "y", "x"} << "y";
}

void TestExecuteCompositeJob::create()
{
    QFETCH(const QStringList, subjobNames);
    QFETCH(const QString, expectedCompositeJobName);

    QList<KJob*> subjobs;
    for (auto& name : subjobNames) {
        subjobs.push_back(new TestJob);
        subjobs.back()->setObjectName(name);
    }
    QCOMPARE(subjobs.size(), subjobNames.size());

    TestCompositeJob compositeJob(nullptr, std::as_const(subjobs));
    QCOMPARE(compositeJob.subjobs(), subjobs);
    QCOMPARE(compositeJob.objectName(), expectedCompositeJobName);
    for (auto& subjob : std::as_const(subjobs)) {
        QCOMPARE(subjob->parent(), &compositeJob);
    }
}

QTEST_GUILESS_MAIN(TestExecuteCompositeJob)

#include "moc_test_executecompositejob.cpp"
