/*
 * Test license header
 * In two lines
 */

#ifndef TESTNAME_H
#define TESTNAME_H

#include <QTest>

class TestName : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void init();
    void cleanup();

    void firstTestCase();
    void secondTestCase();
    void thirdTestCase();
};

#endif // TESTNAME_H
