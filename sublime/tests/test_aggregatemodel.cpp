/***************************************************************************
 *   Copyright 2007 Alexander Dymo  <adymo@kdevelop.org>            *
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
#include "test_aggregatemodel.h"

#include <QtTest/QtTest>
#include <QStandardItem>
#include <QStandardItemModel>

#include <sublime/aggregatemodel.h>
#include <tests/modeltest.h>

using namespace Sublime;

void TestAggregateModel::modelAggregationInASingleView()
{
    AggregateModel *model = new AggregateModel(this);
    model->addModel("First Model", newModel());
    model->addModel("Second Model", newModel());

    //this will assert in case of model problems and the test will fail
    //for detailed explanation why the test failed refer to test/modeltest.cpp
    new ModelTest(model, this);
}

QStandardItemModel * TestAggregateModel::newModel()
{
    /*
    construct the simple model like:
    cool item
    item 0
        item 1
            item 2
                item 3
    */

    QStandardItemModel *model = new QStandardItemModel(this);
    QStandardItem *parentItem = model->invisibleRootItem();

    QStandardItem *item = new QStandardItem(QStringLiteral("cool item"));
    parentItem->appendRow(item);

    for (int i = 0; i < 4; ++i) {
        QStandardItem *item = new QStandardItem(QStringLiteral("item %0").arg(i));
        parentItem->appendRow(item);
        parentItem = item;
    }

    return model;
}


QTEST_MAIN(TestAggregateModel)
