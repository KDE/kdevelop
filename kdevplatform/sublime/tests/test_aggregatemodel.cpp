/*
    SPDX-FileCopyrightText: 2007 Alexander Dymo <adymo@kdevelop.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "test_aggregatemodel.h"

// KDevPlatform
#include <sublime/aggregatemodel.h>
// Qt
#include <QAbstractItemModelTester>
#include <QTest>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QStandardPaths>

using namespace Sublime;

void TestAggregateModel::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
}

void TestAggregateModel::modelAggregationInASingleView()
{
    auto *model = new AggregateModel(this);
    model->addModel(QStringLiteral("First Model"), newModel());
    model->addModel(QStringLiteral("Second Model"), newModel());

    new QAbstractItemModelTester(model, this);
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

    auto *model = new QStandardItemModel(this);
    QStandardItem *parentItem = model->invisibleRootItem();

    auto* item = new QStandardItem(QStringLiteral("cool item"));
    parentItem->appendRow(item);

    for (int i = 0; i < 4; ++i) {
        auto* item = new QStandardItem(QStringLiteral("item %0").arg(i));
        parentItem->appendRow(item);
        parentItem = item;
    }

    return model;
}


QTEST_MAIN(TestAggregateModel)
