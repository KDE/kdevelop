/*
    SPDX-FileCopyrightText: 2011 Silvère Lestang <silvere.lestang@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_TEST_STANDARDOUTPUTVIEW_H
#define KDEVPLATFORM_PLUGIN_TEST_STANDARDOUTPUTVIEW_H

#include <QObject>

#include <shell/uicontroller.h>

namespace KDevelop
{
    class TestCore;
    class IOutputView;
}

namespace Sublime
{
    class View;
    class Controller;
}

class OutputWidget;

class StandardOutputViewTest: public QObject
{
    Q_OBJECT
private:
    OutputWidget* toolViewPointer(const QString& toolViewTitle);
    KDevelop::TestCore* m_testCore;
    KDevelop::IOutputView* m_stdOutputView;
    KDevelop::UiController* m_controller;
    int outputId[5];

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void testRegisterAndRemoveToolView();
    void testActions();
    void testRegisterAndRemoveOutput();
    void testSetModelAndDelegate();
    void testStandardToolViews();
    void testStandardToolViews_data();
};

#endif // KDEVPLATFORM_PLUGIN_TEST_STANDARDOUTPUTVIEW_H
