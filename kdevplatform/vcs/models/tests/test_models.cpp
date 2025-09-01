/*
    SPDX-FileCopyrightText: 2011 Andrey Batyiev <batyiev@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

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
        return VcsFileChangesModel::statusIndexForUrl(*model, QModelIndex{}, url);
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

#include "moc_test_models.cpp"
