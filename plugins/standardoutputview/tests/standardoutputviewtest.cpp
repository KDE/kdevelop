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

#include <QTest>
#include <QtGui>

#include <qtest_kde.h>

#include <tests/testcore.h>
#include <tests/autotestshell.h>
#include <sublime/tooldocument.h>
#include <sublime/view.h>
#include <sublime/area.h>
#include <sublime/controller.h>
#include <sublime/mainwindow.h>

#include "standardoutputviewtest.h"

namespace KDevelop
{
    class IUiController;
}

const QString StandardOutputViewTest::toolviewTitle = "my_toolview";

void StandardOutputViewTest::init()
{
    KDevelop::AutoTestShell::init();
    m_testCore = new KDevelop::TestCore();
    m_testCore->initialize(KDevelop::Core::Default);
    QTest::qWait(500);
     
    m_controller = new KDevelop::UiController(m_testCore);
    m_area = new Sublime::Area( m_controller, "Area" );
    Sublime::MainWindow* mw = new Sublime::MainWindow(m_controller);
    m_controller->showArea(m_area, mw); 
    
}

void StandardOutputViewTest::cleanupTestCase()
{
//     m_testCore->cleanup();
//     delete m_testCore;
    delete m_area;
    delete m_controller;
}

bool StandardOutputViewTest::toolviewExist(QString toolviewTitle)
{
    //KDevelop::UiController* uiController = dynamic_cast<KDevelop::UiController*>(m_testCore->uiController());
    QList< Sublime::View* > views = m_controller->activeArea()->toolViews();
    foreach(Sublime::View* view, views) {
        Sublime::ToolDocument *doc = dynamic_cast<Sublime::ToolDocument*>(view->document());
        //if(doc && doc->title() == name && view->widget()) {
        if(doc)
        {
            if(doc->title() == toolviewTitle) return true;
        }
    }
    return false;
}

void StandardOutputViewTest::testRegisterAndRemoveToolView()
{
    m_stdOutputView = new StandardOutputView(m_testCore);
    
    toolviewId = m_stdOutputView->registerToolView(toolviewTitle, KDevelop::IOutputView::HistoryView, KIcon());
    QVERIFY(toolviewExist(toolviewTitle));
    
    m_stdOutputView->removeToolView(toolviewId);
    QVERIFY(!toolviewExist(toolviewTitle));
}

QTEST_KDEMAIN(StandardOutputViewTest, GUI)