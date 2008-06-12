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

#ifndef QXRUNNER_MODELCREATION_H
#define QXRUNNER_MODELCREATION_H

#include <runneritem.h>
#include <runnermodel.h>
#include <resultsmodel.h>

namespace QxRunner
{
namespace ut
{

using QxRunner::RunnerItem;
using QxRunner::RunnerModel;
using QxRunner::ResultsModel;

class RunnerItemStub : public RunnerItem
{
public:
    RunnerItemStub(const QList<QVariant>& data, RunnerItem* parent)
            : RunnerItem(data, parent) {}

    int run() {
        if (child(0)) {
            setResult(QxRunner::NoResult);  // Have nothing to do as a parent
        } else {
            signalStarted(index());
            setData(1, QString::number(row()) + QString("_1"));
            setData(2, QString::number(row()) + QString("_2"));
            setResult(QxRunner::RunSuccess);
            signalCompleted(index());
        }
        executedItems.push_back(row());
        return result();
    }
    static QList<int> executedItems; // store the index of rows that got executed
};

QList<int> RunnerItemStub::executedItems;

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
        RunnerItemStub* root = new RunnerItemStub(rootData, 0);

        QList<QVariant> columnData;
        columnData << "00" << "01" << "02";
        RunnerItemStub* item1 = new RunnerItemStub(columnData, root);
        item1->setResult(QxRunner::RunSuccess);
        root->appendChild(item1);

        columnData.clear();
        columnData << "10" << "11" << "12";
        RunnerItemStub* item2 = new RunnerItemStub(columnData, root);
        item2->setResult(QxRunner::RunFatal);
        root->appendChild(item2);

        setRootItem(root);
    }

    QString name() const {
        return "";
    }

    RunnerItem* fetchItem(const QModelIndex& index) {
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

#endif // QXRUNNER_MODELCREATION_H
