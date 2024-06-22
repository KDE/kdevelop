/*
    SPDX-FileCopyrightText: 2015 Laszlo Kis-Adam <laszlo.kis-adam@kdemail.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <QTest>
#include <QStandardPaths>
#include <shell/problemstorenode.h>
#include <shell/problem.h>

using namespace KDevelop;

class TestProblemStoreNode : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void testRootNode();
    void testChildren();
    void testLabelNode();
    void testProblemNode();

private:
    QScopedPointer<ProblemStoreNode> m_root;
};


void TestProblemStoreNode::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
    m_root.reset(new ProblemStoreNode());
}

void TestProblemStoreNode::cleanupTestCase()
{
}

void TestProblemStoreNode::testRootNode()
{
    QVERIFY(m_root->isRoot());
    QVERIFY(m_root->parent() == nullptr);
    QCOMPARE(m_root->index(), -1);
    QCOMPARE(m_root->count(), 0);
    QCOMPARE(m_root->children().count(), 0);
    QVERIFY(m_root->label().isEmpty());
    QVERIFY(m_root->problem().constData() == nullptr);
}

void TestProblemStoreNode::testChildren()
{
    QVector<ProblemStoreNode*> nodes;
    nodes.push_back(new ProblemStoreNode());
    nodes.push_back(new ProblemStoreNode());
    nodes.push_back(new ProblemStoreNode());

    int c = 0;
    for (ProblemStoreNode* node : std::as_const(nodes)) {
        m_root->addChild(node);

        c++;
        QCOMPARE(m_root->count(), c);
        QCOMPARE(m_root->children().count(), c);
        QVERIFY(node->parent() == m_root.data());
        QVERIFY(!node->isRoot());
    }

    for (int i = 0; i < c; i++) {
        ProblemStoreNode *node = m_root->child(i);

        QVERIFY(node);
        QVERIFY(node == nodes[i]);
        QCOMPARE(node->index(), i);
    }

    nodes.clear();
    m_root->clear();
    QCOMPARE(m_root->count(), 0);
}

void TestProblemStoreNode::testLabelNode()
{
    QString s1 = QStringLiteral("TEST1");
    QString s2 = QStringLiteral("TEST2");

    auto node = LabelNode(nullptr, s1);
    QCOMPARE(node.label(), s1);

    node.setLabel(s2);
    QCOMPARE(node.label(), s2);
}

void TestProblemStoreNode::testProblemNode()
{
    IProblem::Ptr p1(new DetectedProblem());
    IProblem::Ptr p2(new DetectedProblem());

    QString s1 = QStringLiteral("PROBLEM1");
    QString s2 = QStringLiteral("PROBLEM2");

    p1->setDescription(s1);
    p2->setDescription(s2);

    QScopedPointer<ProblemNode> node;
    node.reset(new ProblemNode(nullptr, p1));

    QCOMPARE(node->problem()->description(), p1->description());

    node->setProblem(p2);
    QCOMPARE(node->problem()->description(), p2->description());
}

QTEST_MAIN(TestProblemStoreNode)

#include "test_problemstorenode.moc"

