/*
    SPDX-FileCopyrightText: 2017 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "test_vcsitemevent.h"

#include <QTest>
#include <QStandardPaths>

#include <vcs/vcsrevision.h>

using namespace KDevelop;

void TestVcsItemEvent::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
}

void TestVcsItemEvent::setItemEvent(VcsItemEvent& itemEvent,
                                    const QString& repositoryLocation,
                                    const QString& repositoryCopySourceLocation,
                                    const VcsRevision& repositoryCopySourceRevision,
                                    VcsItemEvent::Actions actions)
{
    itemEvent.setRepositoryLocation(repositoryLocation);
    itemEvent.setRepositoryCopySourceLocation(repositoryCopySourceLocation);
    itemEvent.setRepositoryCopySourceRevision(repositoryCopySourceRevision);
    itemEvent.setActions(actions);
}

void TestVcsItemEvent::compareItemEvent(const VcsItemEvent& itemEvent,
                                        const QString& repositoryLocation,
                                        const QString& repositoryCopySourceLocation,
                                        const VcsRevision& repositoryCopySourceRevision,
                                        VcsItemEvent::Actions actions)
{
    QCOMPARE(itemEvent.repositoryLocation(), repositoryLocation);
    QCOMPARE(itemEvent.repositoryCopySourceLocation(), repositoryCopySourceLocation);
    QCOMPARE(itemEvent.repositoryCopySourceRevision(), repositoryCopySourceRevision);
    QCOMPARE(itemEvent.actions(), actions);
}

void TestVcsItemEvent::testCopyConstructor()
{
    // test plain copy
    const QString repositoryLocation("location");
    const QString repositoryCopySourceLocation("copy source location");
    VcsRevision repositoryCopySourceRevision;
    repositoryCopySourceRevision.setRevisionValue("A", VcsRevision::GlobalNumber);
    const VcsItemEvent::Actions actions = VcsItemEvent::Added;

    {
        VcsItemEvent itemEventA;
        setItemEvent(itemEventA,
                     repositoryLocation, repositoryCopySourceLocation, repositoryCopySourceRevision, actions);

        VcsItemEvent itemEventB(itemEventA);

        compareItemEvent(itemEventA,
                         repositoryLocation, repositoryCopySourceLocation, repositoryCopySourceRevision,
                         actions);
        compareItemEvent(itemEventB,
                         repositoryLocation, repositoryCopySourceLocation, repositoryCopySourceRevision,
                         actions);
    }

    const QString repositoryLocationNew("new location");

    // test detach after changing A
    {
        VcsItemEvent itemEventA;
        setItemEvent(itemEventA,
                     repositoryLocation, repositoryCopySourceLocation, repositoryCopySourceRevision, actions);

        VcsItemEvent itemEventB(itemEventA);
        // change a property of A
        itemEventA.setRepositoryLocation(repositoryLocationNew);

        compareItemEvent(itemEventA,
                         repositoryLocationNew, repositoryCopySourceLocation, repositoryCopySourceRevision,
                         actions);
        compareItemEvent(itemEventB,
                         repositoryLocation,    repositoryCopySourceLocation, repositoryCopySourceRevision,
                         actions);
    }
}

void TestVcsItemEvent::testAssignOperator()
{
    // test plain assign
    const QString repositoryLocation("location");
    const QString repositoryCopySourceLocation("copy source location");
    VcsRevision repositoryCopySourceRevision;
    repositoryCopySourceRevision.setRevisionValue("A", VcsRevision::GlobalNumber);
    const VcsItemEvent::Actions actions = VcsItemEvent::Added;

    {
        VcsItemEvent itemEventA;
        setItemEvent(itemEventA,
                     repositoryLocation, repositoryCopySourceLocation, repositoryCopySourceRevision, actions);

        VcsItemEvent itemEventB;
        itemEventB = itemEventA;

        compareItemEvent(itemEventA,
                         repositoryLocation, repositoryCopySourceLocation, repositoryCopySourceRevision,
                         actions);
        compareItemEvent(itemEventB,
                         repositoryLocation, repositoryCopySourceLocation, repositoryCopySourceRevision,
                         actions);
    }

    const QString repositoryLocationNew("new location");

    // test detach after changing A
    {
        VcsItemEvent itemEventA;
        setItemEvent(itemEventA,
                     repositoryLocation, repositoryCopySourceLocation, repositoryCopySourceRevision, actions);

        VcsItemEvent itemEventB;
        itemEventB = itemEventA;
        // change a property of A
        itemEventA.setRepositoryLocation(repositoryLocationNew);

        compareItemEvent(itemEventA,
                         repositoryLocationNew, repositoryCopySourceLocation, repositoryCopySourceRevision,
                         actions);
        compareItemEvent(itemEventB,
                         repositoryLocation,    repositoryCopySourceLocation, repositoryCopySourceRevision,
                         actions);
    }
}

QTEST_GUILESS_MAIN(TestVcsItemEvent)

#include "moc_test_vcsitemevent.cpp"
