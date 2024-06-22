/*
    SPDX-FileCopyrightText: 2015 Laszlo Kis-Adam <laszlo.kis-adam@kdemail.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <QTest>
#include <QSignalSpy>
#include <QVector>
#include "shell/problemmodelset.h"
#include "shell/problemmodel.h"
#include "tests/testcore.h"
#include "tests/autotestshell.h"

using namespace KDevelop;

namespace
{

const int testModelCount = 3;

const QString testModelIds[] = {
    QStringLiteral("MODEL1_ID"),
    QStringLiteral("MODEL2_ID"),
    QStringLiteral("MODEL3_ID")
};

const QString testModelNames[] = {
    QStringLiteral("MODEL1"),
    QStringLiteral("MODEL2"),
    QStringLiteral("MODEL3")
};

struct TestModelData
{
    QString id;
    QString name;
    ProblemModel* model;
};

}

class TestProblemModelSet : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void testAddModel();
    void testFindModel();
    void testModelListing();
    void testRemoveModel();

private:
    QScopedPointer<ProblemModelSet> m_set;
    QVector<TestModelData> m_testData;

};

void TestProblemModelSet::initTestCase()
{
    AutoTestShell::init();
    TestCore::initialize(Core::NoUi);

    m_set.reset(new ProblemModelSet());

    for (int i = 0; i < testModelCount; i++) {
        m_testData.push_back(TestModelData({testModelIds[i], testModelNames[i], new ProblemModel(nullptr)}));
    }
}

void TestProblemModelSet::cleanupTestCase()
{
    for (auto& d : std::as_const(m_testData)) {
        delete d.model;
    }

    m_testData.clear();

    TestCore::shutdown();
}

void TestProblemModelSet::testAddModel()
{
    QSignalSpy spy(m_set.data(), &ProblemModelSet::added);

    int c = 0;
    for (int i = 0; i < testModelCount; i++) {
        m_set->addModel(m_testData[i].id, m_testData[i].name, m_testData[i].model);
        c++;
        QCOMPARE(spy.count(), c);
        QCOMPARE(m_set->models().count(), c);
    }

}

void TestProblemModelSet::testFindModel()
{
    ProblemModel *model = nullptr;
    for (int i = 0; i < testModelCount; i++) {
        model = m_set->findModel(m_testData[i].id);

        QVERIFY(model);
        QVERIFY(model == m_testData[i].model);
    }

    model = m_set->findModel(QString());
    QVERIFY(model == nullptr);

    model = m_set->findModel(QStringLiteral("RandomName"));
    QVERIFY(model == nullptr);
}

void TestProblemModelSet::testModelListing()
{
    QVector<ModelData> models = m_set->models();
    QCOMPARE(models.size(), testModelCount);

    for (int i = 0; i < testModelCount; i++) {
        QCOMPARE(models[i].name, m_testData[i].name);
        QCOMPARE(models[i].model, m_testData[i].model);
    }
}

void TestProblemModelSet::testRemoveModel()
{
    QSignalSpy spy(m_set.data(), &ProblemModelSet::removed);

    int c = 0;
    for (const TestModelData& data : std::as_const(m_testData)) {
        m_set->removeModel(data.id);
        c++;

        QCOMPARE(spy.count(), c);
        QVERIFY(testModelCount >= c);
        QCOMPARE(m_set->models().count(), testModelCount - c);
    }
}

QTEST_MAIN(TestProblemModelSet)

#include "test_problemmodelset.moc"
