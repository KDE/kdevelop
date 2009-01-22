#include "%{APPNAME}Test.h"
#include <QtTest/QTest>

void %{APPNAME}Test::initTestCase()
{}

void %{APPNAME}Test::init()
{}

void %{APPNAME}Test::cleanup()
{}

void %{APPNAME}Test::cleanupTestCase()
{}

void %{APPNAME}Test::someTest()
{
    QCOMPARE(1,2);
}


QTEST_MAIN(%{APPNAME}Test)
#include "%{APPNAME}Test.moc"
