/*
    SPDX-FileCopyrightText: 2017 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_TESTVCSITEMEVENT_H
#define KDEVPLATFORM_TESTVCSITEMEVENT_H

#include <QObject>
#include <vcs/vcsevent.h>

namespace KDevelop {
class VcsRevision;
}
class QString;

class TestVcsItemEvent : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void testCopyConstructor();
    void testAssignOperator();

private:
    void setItemEvent(KDevelop::VcsItemEvent& itemEvent,
                      const QString& repositoryLocation,
                      const QString& repositoryCopySourceLocation,
                      const KDevelop::VcsRevision& repositoryCopySourceRevision,
                      KDevelop::VcsItemEvent::Actions actions);
    void compareItemEvent(const KDevelop::VcsItemEvent& itemEvent,
                          const QString& repositoryLocation,
                          const QString& repositoryCopySourceLocation,
                          const KDevelop::VcsRevision& repositoryCopySourceRevision,
                          KDevelop::VcsItemEvent::Actions actions);
};

#endif // KDEVPLATFORM_TESTVCSITEMEVENT_H
