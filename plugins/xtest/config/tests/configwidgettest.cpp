/*
 * This file is part of KDevelop
 * Copyright 2008 Manuel Breugelmans <mbr.nxi@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include "configwidgettest.h"
#include <QtTest/QTest>
#include <qtest_kde.h>
#include "../../kasserts.h"
#include "../configwidget.h"
#include "fakedetailswidget.h"
#include <QLabel>
#include <KUrlRequester>

using Veritas::ConfigWidgetTest;
using Veritas::ConfigWidget;

void ConfigWidgetTest::init()
{
    m_config = new ConfigWidget;
    m_config->setDetailsWidget(new FakeDetailsWidget());
    m_config->show();
}

void ConfigWidgetTest::cleanup()
{
    delete m_config;
}

// command
void ConfigWidgetTest::construct()
{
    // this allows you to play with the widget. adjust wait time.
    //QTest::qWait(5000);
}

// custom assertion
void ConfigWidgetTest::assertDetailsCollapsed()
{
    QList<FakeDetailsWidget*> fdw = m_config->findChildren<FakeDetailsWidget*>();
    Q_ASSERT(!fdw.isEmpty());
    KVERIFY(fdw[0]->isHidden());
}

// custom assertion
void ConfigWidgetTest::assertDetailsExpanded()
{
    QList<FakeDetailsWidget*> fdw = m_config->findChildren<FakeDetailsWidget*>();
    Q_ASSERT(!fdw.isEmpty());
    KVERIFY(fdw[0]->isVisible());
}

// command
void ConfigWidgetTest::detailsWidgetHiddenByDefault()
{
    // the details widget should not be visible on construction

    assertDetailsCollapsed();
}

// command
void ConfigWidgetTest::expandDetailsWidget()
{
    // click once on the expand detail button
    // the detail widget should be visible

    m_config->fto_clickExpandDetails();
    assertDetailsExpanded();
}

// command
void ConfigWidgetTest::collapseDetailsWidget()
{
    // click twice on the expand detail button
    // now the details widget should be hidden again.

    m_config->fto_clickExpandDetails();
    m_config->fto_clickExpandDetails();
    assertDetailsCollapsed();
}

// custom assertion
void ConfigWidgetTest::assertTestExeFieldsShown(int numberOf)
{
    QList<KUrlRequester*> fdw = m_config->findChildren<KUrlRequester*>();
    KOMPARE(numberOf, fdw.count());
}

// command
void ConfigWidgetTest::noTestExeFieldShownByDefault()
{
    // by default no test-executable url bar should be shown

    assertTestExeFieldsShown(0);
}

// command
void ConfigWidgetTest::addTestExeField()
{
    // add a test-executable url-bar. the configwidget should now contain 1

    m_config->fto_clickAddTestExeField();
    assertTestExeFieldsShown(1);
}

// command
void ConfigWidgetTest::addTwoTestExeFields()
{
    // add a second and third test-executable url-bar. the configwidget
    // should now contain 2 of them.

    m_config->fto_clickAddTestExeField();
    m_config->fto_clickAddTestExeField();
    assertTestExeFieldsShown(2);
}

// command
void ConfigWidgetTest::removeFirstTestExeField()
{
    // remove the single present (by default) test executable field
    // now the config widget should contain zero test-exe url bars.

    m_config->fto_clickAddTestExeField();
    m_config->fto_clickRemoveTestExeField(0); // the index of the field to remove
    assertTestExeFieldsShown(0);
}

// command
void ConfigWidgetTest::removeSecondTestExeField()
{
    // first add a second test exe bar, then remove it again.
    // now the config widget should contain a single test-exe url bar

    m_config->fto_clickAddTestExeField();
    m_config->fto_clickAddTestExeField();
    m_config->fto_clickRemoveTestExeField(1); // the index of the field to remove
    assertTestExeFieldsShown(1);
}

// command
void ConfigWidgetTest::removeAllTestExeFieldsThenReAdd()
{
    // first add a second and third test exe bar. Then remove all 3 currently
    // present test exe bars. Finally add them again.
    // the configwidget should now contain 3 test exe bars.

    // add
    m_config->fto_clickAddTestExeField();
    m_config->fto_clickAddTestExeField();
    m_config->fto_clickAddTestExeField();

    // remove
    m_config->fto_clickRemoveTestExeField(0);
    m_config->fto_clickRemoveTestExeField(0);
    m_config->fto_clickRemoveTestExeField(0);

    // re-add
    m_config->fto_clickAddTestExeField();
    m_config->fto_clickAddTestExeField();
    m_config->fto_clickAddTestExeField();

    assertTestExeFieldsShown(3);
}

void ConfigWidgetTest::noFrameworksComboBoxByDefault()
{
    // the test-framework selection box should be empty
    // by default

    QStringList frameworks = m_config->fto_frameworkComboBoxContents();
    KVERIFY(frameworks.isEmpty());
}

void ConfigWidgetTest::singleFrameworkComboBox()
{
    // append a single framework to the box

    m_config->appendFramework("FooBar");
    QStringList frameworks = m_config->fto_frameworkComboBoxContents();
    KOMPARE(1, frameworks.count());
    KOMPARE(QString("FooBar"), frameworks[0]);
}

void ConfigWidgetTest::multipleFrameworksComboBox()
{
    // append multiple frameworks to the selection box

    m_config->appendFramework("Foo");
    m_config->appendFramework("Bar");
    QStringList frameworks = m_config->fto_frameworkComboBoxContents();
    KOMPARE(2, frameworks.count());
    KVERIFY(frameworks.contains("Foo"));
    KVERIFY(frameworks.contains("Bar"));
}

void ConfigWidgetTest::executableContents()
{
    // add test executable bars with actual urls filled
    // these urls should now be available through 'executables()'

    m_config->addTestExecutableField(KUrl("/foo"));
    m_config->addTestExecutableField(KUrl("http://bar"));

    KUrl::List executables = m_config->executables();
    KOMPARE(2, executables.count());
    KVERIFY(executables.contains(KUrl("/foo")));
    KVERIFY(executables.contains(KUrl("http://bar")));
}

QTEST_KDEMAIN( ConfigWidgetTest, GUI )
#include "configwidgettest.moc"

