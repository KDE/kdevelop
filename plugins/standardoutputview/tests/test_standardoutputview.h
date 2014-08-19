/*
    Copyright (C) 2011  Silvère Lestang <silvere.lestang@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/


#ifndef KDEVPLATFORM_PLUGIN_TEST_STANDARDOUTPUTVIEW_H
#define KDEVPLATFORM_PLUGIN_TEST_STANDARDOUTPUTVIEW_H

#include <QtCore/QObject>

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
    OutputWidget* toolviewPointer(QString toolviewTitle);
    KDevelop::TestCore* m_testCore;
    KDevelop::IOutputView* m_stdOutputView;
    KDevelop::UiController* m_controller;
    int toolviewId;
    int outputId[5];
    static const QString toolviewTitle;

private slots:
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
