/*
    SPDX-FileCopyrightText: 2007 Alexander Dymo <adymo@kdevelop.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_TEST_AGGREGATEMODEL_H
#define KDEVPLATFORM_TEST_AGGREGATEMODEL_H

#include <QObject>

class QStandardItemModel;

class TestAggregateModel: public QObject {
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void modelAggregationInASingleView();

private:
    QStandardItemModel *newModel();

};

#endif
