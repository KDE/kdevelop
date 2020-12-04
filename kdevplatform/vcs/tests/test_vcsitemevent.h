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
