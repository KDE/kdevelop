/* KDevelop xUnit plugin
 *
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

#include "resultswidgettest.h"
#include "../internal/resultswidget.h"
#include <qtest_kde.h>
#include <QStandardItemModel>

using Veritas::ResultsWidget;
using Veritas::ResultsWidgetTest;

void ResultsWidgetTest::init()
{
}

void ResultsWidgetTest::cleanup()
{
}

void ResultsWidgetTest::construct()
{
    // The resultwidget constructor triggered an assert in QHeaderView,
    // hence this test.
    // It only verifies that Qt doesn't assert out.

    ResultsWidget* rw = new ResultsWidget; // should not crash

    // now initialize a model with 4 columns
    QStandardItemModel* model = new QStandardItemModel;
    model->setHorizontalHeaderLabels(QStringList() << "col1" << "col2" << "col3" << "col4");
    rw->tree()->setModel(model);

    rw->setResizeMode(); // setResizeMode() should not crash either.

    delete rw;
    delete model;
}

QTEST_KDEMAIN( ResultsWidgetTest, GUI )
#include "resultswidgettest.moc"
