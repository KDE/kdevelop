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
#include "areawalkertest.h"

#include <QStringList>
#include <QtTest/QtTest>

#include <sublime/area.h>
#include <sublime/view.h>
#include <sublime/controller.h>
#include <sublime/partdocument.h>

#include "kdevtest.h"
#include "areaprinter.h"

using namespace Sublime;

struct AreaStopper {
    AreaStopper(QString stopAt): m_stopAt(stopAt) {}
    Area::WalkerMode operator()(AreaIndex *index)
    {
        foreach (View *view, index->views())
        {
            list << view->objectName();
            if (view->objectName() == m_stopAt)
                return Area::StopWalker;
        }
        return Area::ContinueWalker;
    }
    QStringList list;
    QString m_stopAt;
};

void AreaWalkerTest::testWalkerModes()
{
    Controller *controller = new Controller(this);
    Document *doc = new PartDocument(controller, KUrl::fromPath("~/foo.cpp"));
    Area *area = new Area(controller, "Area");
    View *view1 = doc->createView();
    view1->setObjectName("1");
    area->addView(view1);
    View *view2 = doc->createView();
    view2->setObjectName("2");
    area->addView(view2, view1, Qt::Vertical);
    View *view3 = doc->createView();
    view3->setObjectName("3");
    area->addView(view3, view1, Qt::Vertical);
    View *view4 = doc->createView();
    view4->setObjectName("4");
    area->addView(view4, view1, Qt::Vertical);

    AreaViewsPrinter p;
    area->walkViews(p, area->rootIndex());
    QCOMPARE(p.result, QString("\n\
[ vertical splitter]\n\
    [ vertical splitter]\n\
        [ vertical splitter]\n\
            [ 1 ]\n\
            [ 4 ]\n\
        [ 3 ]\n\
    [ 2 ]\n\
"));

    AreaStopper stopper("1");
    area->walkViews(stopper, area->rootIndex());
    QCOMPARE(stopper.list.join(" "), QString("1"));

    AreaStopper stopper2("2");
    area->walkViews(stopper2, area->rootIndex());
    QCOMPARE(stopper2.list.join(" "), QString("1 4 3 2"));

    AreaStopper stopper3("3");
    area->walkViews(stopper3, area->rootIndex());
    QCOMPARE(stopper3.list.join(" "), QString("1 4 3"));

    AreaStopper noStopper("X");
    area->walkViews(noStopper, area->rootIndex());
    QCOMPARE(noStopper.list.join(" "), QString("1 4 3 2"));

    delete area;
    delete doc;
    delete controller;
}

KDEVTEST_MAIN(AreaWalkerTest)
#include "areawalkertest.moc"
