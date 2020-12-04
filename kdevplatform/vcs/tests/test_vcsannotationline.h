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

#ifndef KDEVPLATFORM_TESTVCSANNOTATIONLINE_H
#define KDEVPLATFORM_TESTVCSANNOTATIONLINE_H

#include <QObject>

namespace KDevelop {
class VcsAnnotationLine;
class VcsRevision;
}
class QDateTime;
class QString;

class TestVcsAnnotationLine : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void testCopyConstructor();
    void testAssignOperator();

private:
    void setAnnotationLine(KDevelop::VcsAnnotationLine& annotationLine,
                           int lineNumber,
                           const QString& text,
                           const QString& author,
                           const KDevelop::VcsRevision& revision,
                           const QDateTime& date,
                           const QString& commitMessage);
    void compareAnnotationLine(const KDevelop::VcsAnnotationLine& annotationLine,
                               int lineNumber,
                               const QString& text,
                               const QString& author,
                               const KDevelop::VcsRevision& revision,
                               const QDateTime& date,
                               const QString& commitMessage);
};

#endif // KDEVPLATFORM_TESTVCSANNOTATIONLINE_H
