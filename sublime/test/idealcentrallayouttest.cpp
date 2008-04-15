/***************************************************************************
 *   Copyright 2006-2007 Alexander Dymo  <adymo@kdevelop.org>       *
 *   Copyright 2008 Hamish Rodda <rodda@kde.org>                           *
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
#include "idealcentrallayouttest.h"

#include <QtTest/QtTest>

#include <QDockWidget>
#include <QListView>
#include <QTextEdit>
#include <QSplitter>

#include <kdebug.h>
#include <kapplication.h>
#include <ktextedit.h>
#include <kactioncollection.h>

#include <sublime/view.h>
#include <sublime/area.h>
#include <sublime/sublimedefs.h>
#include <sublime/tooldocument.h>
#include <sublime/urldocument.h>
#include <sublime/controller.h>
#include <sublime/mainwindow.h>

#include "ideal.h"
#include "idealcentralwidget.h"

#include "idealcentrallayout.h"

#include "kdevtest.h"
#include "areaprinter.h"

using namespace Sublime;

void IdealCentralLayoutTest::init()
{
    m_controller = new Controller(this);
    Document *doc1 = new UrlDocument(m_controller, KUrl::fromPath("~/foo.cpp"));
    Document *doc2 = new UrlDocument(m_controller, KUrl::fromPath("~/boo.cpp"));
    Document *doc3 = new UrlDocument(m_controller, KUrl::fromPath("~/moo.cpp"));
    Document *doc4 = new UrlDocument(m_controller, KUrl::fromPath("~/zoo.cpp"));

    //areas (aka perspectives)
    //view object names are in form AreaNumber.DocumentNumber.ViewNumber
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
}

void IdealCentralLayoutTest::cleanup()
{
    delete m_area1;
    delete m_area2;
    delete m_controller;
    m_area1 = 0;
    m_area2 = 0;
    m_controller = 0;
}


void IdealCentralLayoutTest::testLayoutConstruction()
{
    MainWindow mw1(m_controller);
    KActionCollection ac(this);
    IdealMainWidget* imw = new IdealMainWidget(&mw1, &ac);
    m_cw = new IdealCentralWidget(imw);
    m_controller->showArea(m_area1, &mw1);
    checkArea1(&mw1);
}

void IdealCentralLayoutTest::checkArea1(MainWindow *mw)
{
    AreaLayout* topLayout = m_cw->centralLayout()->topLayout();
    QVERIFY(topLayout);
}

KDEVTEST_MAIN(IdealCentralLayoutTest)
#include "idealcentrallayouttest.moc"

