#include "configwidgettest.h"
#include <QtTest/QTest>
#include <qtest_kde.h>
#include <QLabel>

#include "../configwidget.h"

using Veritas::ConfigWidgetTest;

void ConfigWidgetTest::init()
{
}

void ConfigWidgetTest::cleanup()
{
}

void ConfigWidgetTest::construct()
{
    ConfigWidget* w = new ConfigWidget;
    w->show();
    QTest::qWait(5000);
}

void ConfigWidgetTest::expandDetails()
{
    ConfigWidget* w = new ConfigWidget;
    QList<QLabel*> labels = w->findChildren<QLabel*>();
    QVERIFY(labels.isEmpty());
    w->fto_clickExpandDetails();
    w->show();
    labels = w->findChildren<QLabel*>();
    QVERIFY(!labels.isEmpty());
}

QTEST_KDEMAIN( ConfigWidgetTest, GUI )
#include "configwidgettest.moc"
