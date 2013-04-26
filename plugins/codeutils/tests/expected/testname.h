/*
 * Test license header
 * In two lines
 */

#ifndef KDEVPLATFORM_PLUGIN_TESTNAME_H
#define KDEVPLATFORM_PLUGIN_TESTNAME_H

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

#endif // KDEVPLATFORM_PLUGIN_TESTNAME_H
