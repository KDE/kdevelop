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

#ifndef KDEVPLATFORM_TESTVCSDIFF_H
#define KDEVPLATFORM_TESTVCSDIFF_H

#include <QObject>

#include "vcsdiff.h"

class TestVcsDiff : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void testCopyConstructor();
    void testAssignOperator();

private:
    void setDiff(KDevelop::VcsDiff& diff,
                 const QString& diffString,
                 const QUrl& baseDiff,
                 uint depth);
    void compareDiff(const KDevelop::VcsDiff& diff,
                     const QString& diffString,
                     const QUrl& baseDiff,
                     uint depth);
};

#endif // KDEVPLATFORM_TESTVCSDIFF_H
