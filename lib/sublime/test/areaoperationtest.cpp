/***************************************************************************
 *   Copyright (C) 2006-2007 by Alexander Dymo  <adymo@kdevelop.org>       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/
#include "areaoperationtest.h"

#include <QtTest/QtTest>

#include <QListView>
#include <QTextEdit>

#include <kdebug.h>

#include <sublime/view.h>
#include <sublime/area.h>
#include <sublime/sublimedefs.h>
#include <sublime/tooldocument.h>
#include <sublime/partdocument.h>
#include <sublime/controller.h>
#include <sublime/mainwindow.h>

#include "areaprinter.h"

using namespace Sublime;

void AreaOperationTest::init()
{
    m_controller = new Controller(this);
    Document *doc1 = new PartDocument(m_controller, KUrl::fromPath("~/foo.cpp"));
    doc1->setObjectName("doc1");
    Document *doc2 = new PartDocument(m_controller, KUrl::fromPath("~/boo.cpp"));
    doc2->setObjectName("doc2");
    Document *doc3 = new PartDocument(m_controller, KUrl::fromPath("~/moo.cpp"));
    doc3->setObjectName("doc3");
    Document *doc4 = new PartDocument(m_controller, KUrl::fromPath("~/zoo.cpp"));
    doc4->setObjectName("doc4");

    //documents for toolviews
    Document *tool1 = new ToolDocument(m_controller, new SimpleToolWidgetFactory<QListView>());
    tool1->setObjectName("tool1");
    Document *tool2 = new ToolDocument(m_controller, new SimpleToolWidgetFactory<QTextEdit>());
    tool2->setObjectName("tool2");
    Document *tool3 = new ToolDocument(m_controller, new SimpleToolWidgetFactory<QTextEdit>());
    tool2->setObjectName("tool3");

    //areas (aka perspectives)
    //view object names are in form AreaNumber.DocumentNumber.ViewNumber
    //"tool" prefix is there for tooldocument views
    m_area1 = new Area(m_controller, "Area 1");
    View *view = doc1->createView();
    view->setObjectName("view1.1.1");
    m_area1->addView(view);
    view = doc2->createView();
    view->setObjectName("view1.2.1");
    m_area1->addView(view);
    view = doc2->createView();
    view->setObjectName("view1.2.2");
    m_area1->addView(view);
    view = doc3->createView();
    view->setObjectName("view1.3.1");
    m_area1->addView(view);
    view = tool1->createView();
    view->setObjectName("toolview1.1.1");
    m_area1->addToolView(view, Sublime::Left);
    view = tool2->createView();
    view->setObjectName("toolview1.2.1");
    m_area1->addToolView(view, Sublime::Bottom);
    view = tool2->createView();
    view->setObjectName("toolview1.2.2");
    m_area1->addToolView(view, Sublime::Bottom);

    m_area2 = new Area(m_controller, "Area 2");
    View *view211 = doc1->createView();
    view211->setObjectName("view2.1.1");
    m_area2->addView(view211);
    View *view212 = doc1->createView();
    view212->setObjectName("view2.1.2");
    m_area2->addView(view212);
    View *view221 = doc2->createView();
    view221->setObjectName("view2.2.1");
    m_area2->addView(view221, view211, Qt::Vertical);
    View *view231 = doc3->createView();
    view231->setObjectName("view2.3.1");
    m_area2->addView(view231, view221, Qt::Horizontal);
    View *view241 = doc4->createView();
    view241->setObjectName("view2.4.1");
    m_area2->addView(view241, view212, Qt::Vertical);
    view = tool1->createView();
    view->setObjectName("toolview2.1.1");
    m_area2->addToolView(view, Sublime::Bottom);
    view = tool2->createView();
    view->setObjectName("toolview2.2.1");
    m_area2->addToolView(view, Sublime::Right);
    view = tool3->createView();
    view->setObjectName("toolview2.3.1");
    m_area2->addToolView(view, Sublime::Top);
    view = tool3->createView();
    view->setObjectName("toolview2.3.2");
    m_area2->addToolView(view, Sublime::Top);
}

void AreaOperationTest::cleanup()
{
    delete m_area1;
    delete m_area2;
    delete m_controller;
    m_area1 = 0;
    m_area2 = 0;
    m_controller = 0;
}

void AreaOperationTest::testAreaConstruction()
{
    //check if areas has proper object names
    QCOMPARE(m_area1->objectName(), QString("Area 1"));
    QCOMPARE(m_area2->objectName(), QString("Area 2"));

    //check that area1 contents is properly initialised
    AreaViewsPrinter viewsPrinter1;
    m_area1->walkViews(viewsPrinter1, m_area1->rootIndex());
    QCOMPARE(viewsPrinter1.result, QString("\n\
view1.1.1\n\
view1.2.1\n\
view1.2.2\n\
view1.3.1\n\
"));
    AreaToolViewsPrinter toolViewsPrinter1;
    m_area1->walkToolViews(toolViewsPrinter1, Sublime::AllPositions);
    QCOMPARE(toolViewsPrinter1.result, QString("\n\
toolview1.1.1\n\
toolview1.2.1\n\
toolview1.2.2\n\
"));

    //check that area2 contents is properly initialised
    AreaViewsPrinter viewsPrinter2;
    m_area2->walkViews(viewsPrinter2, m_area2->rootIndex());
    QCOMPARE(viewsPrinter2.result, QString("\n\
view2.1.1\n\
view2.1.2\n\
view2.4.1\n\
view2.2.1\n\
view2.3.1\n\
"));
    AreaToolViewsPrinter toolViewsPrinter2;
    m_area2->walkToolViews(toolViewsPrinter2, Sublime::AllPositions);
    QCOMPARE(toolViewsPrinter2.result, QString("\n\
toolview2.1.1\n\
toolview2.2.1\n\
toolview2.3.1\n\
toolview2.3.2\n\
"));
}

void AreaOperationTest::testAreaCloning()
{

}

void AreaOperationTest::testAreaSwitchingInSameMainwindow()
{

}


QTEST_MAIN(AreaOperationTest)
#include "areaoperationtest.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
