/***************************************************************************
 *   Copyright 2011 Andrey Batyiev <batyiev@gmail.com>                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU General Public License as        *
 *   published by the Free Software Foundation; either version 2 of        *
 *   the License or (at your option) version 3 or any later version        *
 *   accepted by the membership of KDE e.V. (or its successor approved     *
 *   by the membership of KDE e.V.), which shall act as a proxy            *
 *   defined in Section 14 of version 3 of the license.                    *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 ***************************************************************************/

#include "test_models.h"

#include <QTest>

#include <vcs/models/vcsfilechangesmodel.h>
#include <tests/autotestshell.h>
#include <tests/testcore.h>

using namespace KDevelop;

void TestModels::initTestCase()
{
    AutoTestShell::init({QStringLiteral("dummy")});
    TestCore::initialize();
}

void TestModels::cleanupTestCase()
{
    TestCore::shutdown();
}

void TestModels::testVcsFileChangesModel()
{
    const auto indexForUrl = [](const VcsFileChangesModel* model, const QUrl& url) {
        return model->match(model->index(0, 0), VcsFileChangesModel::UrlRole,
                            url, 1, Qt::MatchExactly).value(0);
    };
    const auto statusInfo = [](const QModelIndex& idx) {
        return idx.data(VcsFileChangesModel::VcsStatusInfoRole).value<VcsStatusInfo>();
    };

    QScopedPointer<VcsFileChangesModel> model(new VcsFileChangesModel);

    // Newly created model should be empty
    QVERIFY(model->rowCount() == 0);

    // Pull some files into
    QUrl filenames[] = {
        QUrl::fromLocalFile(QStringLiteral("foo")),
        QUrl::fromLocalFile(QStringLiteral("bar")),
        QUrl::fromLocalFile(QStringLiteral("pew")),
        QUrl::fromLocalFile(QStringLiteral("trash"))
    };
    VcsStatusInfo::State states[] = {VcsStatusInfo::ItemAdded, VcsStatusInfo::ItemModified, VcsStatusInfo::ItemDeleted, VcsStatusInfo::ItemUpToDate};
    VcsStatusInfo status;

    for(int i = 0; i < 3; i++) {
        status.setUrl(filenames[i]);
        status.setState(states[i]);
        model->updateState(status);
        QCOMPARE(model->rowCount(), (i+1));
    }

    // Pulling up-to-date file doesn't change anything
    {
        status.setUrl(filenames[3]);
        status.setState(states[3]);
        model->updateState(status);
        QCOMPARE(model->rowCount(), 3);
    }

    // Check that all OK
    for(int i = 0; i < 3; i++) {
        QModelIndex idx = indexForUrl(model.data(), filenames[i]);
        QVERIFY(idx.isValid());
        VcsStatusInfo info = statusInfo(idx);
        QVERIFY(info.url().isValid());
        QCOMPARE(info.url(), filenames[i]);
        QCOMPARE(info.state(), states[i]);
    }

    // Pull it all again = nothing changed
    for(int i = 0; i < 3; i++) {
        status.setUrl(filenames[i]);
        status.setState(states[i]);
        model->updateState(status);
        QCOMPARE(model->rowCount(), 3);
    }

    // Check that all OK
    for(int i = 0; i < 3; i++) {
        QModelIndex item = indexForUrl(model.data(), filenames[i]);
        QVERIFY(item.isValid());
        VcsStatusInfo info = statusInfo(item);
        QCOMPARE(info.url(), filenames[i]);
        QCOMPARE(info.state(), states[i]);
    }

    // Remove one file
    {
        states[1] = VcsStatusInfo::ItemUpToDate;
        status.setUrl(filenames[1]);
        status.setState(states[1]);
        model->updateState(status);
        QCOMPARE(model->rowCount(), 2);
    }

    // Check them all
    for(int i = 0; i < 3; i++) {
        if(states[i] != VcsStatusInfo::ItemUpToDate && states[i] != VcsStatusInfo::ItemUnknown) {
            QModelIndex item = indexForUrl(model.data(), filenames[i]);
            QVERIFY(item.isValid());
            VcsStatusInfo info = statusInfo(item);
            QCOMPARE(info.url(), filenames[i]);
            QCOMPARE(info.state(), states[i]);
        }
    }

    // Delete them all
    model->removeRows(0, model->rowCount());
    QCOMPARE(model->rowCount(), 0);

    // Pull it all again
    for(int i = 0; i < 3; i++) {
        status.setUrl(filenames[i]);
        status.setState(states[i]);
        model->updateState(status);
    }
    QCOMPARE(model->rowCount(), 2);
}

QTEST_MAIN(TestModels)
