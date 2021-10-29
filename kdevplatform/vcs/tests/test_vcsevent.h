/*
    SPDX-FileCopyrightText: 2017 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_TESTVCSEVENT_H
#define KDEVPLATFORM_TESTVCSEVENT_H

#include <QObject>
#include <QList>
#include <vcs/vcsevent.h>

namespace KDevelop {
class VcsRevision;
}
class QDateTime;
class QString;

class TestVcsEvent : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void testCopyConstructor();
    void testAssignOperator();

private:
    void setEvent(KDevelop::VcsEvent& event,
                  const KDevelop::VcsRevision& revision,
                  const QString& author,
                  const QDateTime& date,
                  const QString& message,
                  const QList<KDevelop::VcsItemEvent>& items);
    void compareEvent(const KDevelop::VcsEvent& event,
                      const KDevelop::VcsRevision& revision,
                      const QString& author,
                      const QDateTime& date,
                      const QString& message,
                      const QList<KDevelop::VcsItemEvent>& items);
};

#endif // KDEVPLATFORM_TESTVCSEVENT_H
