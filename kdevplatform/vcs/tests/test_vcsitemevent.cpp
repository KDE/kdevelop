/* This file is part of KDevelop
 *
 * Copyright 2017 Friedrich W. H. Kossebau <kossebau@kde.org>
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

#include "test_vcsitemevent.h"

#include <QTest>

#include <vcs/vcsrevision.h>

using namespace KDevelop;

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
