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

#ifndef VERITAS_MODELCREATION_H
#define VERITAS_MODELCREATION_H

#include <test.h>
#include <runnermodel.h>
#include <resultsmodel.h>

namespace Veritas
{
namespace ut
{

using Veritas::Test;
using Veritas::RunnerModel;
using Veritas::ResultsModel;

class TestStub : public Test
{
public:
    TestStub(const QList<QVariant>& data, Test* parent)
            : Test(data, parent) {}

    int run() {
        if (child(0)) {
            setState(Veritas::NoResult);  // Have nothing to do as a parent
        } else {
            started();
            setData(1, QString::number(row()) + QString("_1"));
            setData(2, QString::number(row()) + QString("_2"));
            setState(Veritas::RunSuccess);
            finished();
        }
        executedItems.push_back(row());
        return state();
    }
    bool shouldRun() const {
        return true;
    }
    static QList<int> executedItems; // store the index of rows that got executed
};

QList<int> TestStub::executedItems;

class RunnerModelStub : public RunnerModel
{
public:
    RunnerModelStub(bool fill = true)
            : RunnerModel(NULL),
            col0Caption("run_col0"),
            col1Caption("run_col1"),
            col2Caption("run_col2") {
        if (fill) this->fill();
    }

    void fill() {
        QList<QVariant> rootData;
        rootData << col0Caption << col1Caption << col2Caption;
        TestStub* root = new TestStub(rootData, 0);

        QList<QVariant> columnData;
        columnData << "00" << "01" << "02";
        TestStub* item1 = new TestStub(columnData, root);
        item1->setState(Veritas::RunSuccess);
        root->addChild(item1);

        columnData.clear();
        columnData << "10" << "11" << "12";
        TestStub* item2 = new TestStub(columnData, root);
        item2->setState(Veritas::RunFatal);
        root->addChild(item2);

        setRootItem(root);
    }

    QString name() const {
        return "";
    }

    Test* fetchItem(const QModelIndex& index) {
        return itemFromIndex(index);
    }

    void decapitate() {
        setRootItem(0);
    }

    // column headers
    QVariant col0Caption;
    QVariant col1Caption;
    QVariant col2Caption;
};

inline RunnerModelStub* createRunnerModelStub(bool fill = true)
{
    return new RunnerModelStub(fill);
}

class ResultsHeader
{
public:
    static const QString col0;
    static const QString col1;
    static const QString col2;
};

const QString ResultsHeader::col0 = "col0";
const QString ResultsHeader::col1 = "col1";
const QString ResultsHeader::col2 = "col2";

inline ResultsModel* createResultsModelStub()
{
    QStringList header;
    header << ResultsHeader::col0 << ResultsHeader::col1 << ResultsHeader::col2;

    ResultsModel* model = new ResultsModel(header);
    RunnerModelStub* runnerModel = new RunnerModelStub();

    model->addResult(runnerModel->index(0, 0)); // invoke slot
    model->addResult(runnerModel->index(1, 0)); // invoke slot

    return model;
}

}
}

#endif // VERITAS_MODELCREATION_H
