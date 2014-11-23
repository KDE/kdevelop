/*
 * Test license header
 * In two lines
 */

#ifndef TESTNAME_H
#define TESTNAME_H

#include <QtTest/QtTest>

class TestName : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void cleanupTestCase();

    void init();
    void cleanup();

    void firstTestCase();
    void secondTestCase();
    void thirdTestCase();
};

#endif // TESTNAME_H
