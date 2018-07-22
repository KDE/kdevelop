/* This file is part of KDevelop
 *
 * Copyright 2018 Friedrich W. H. Kossebau <kossebau@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#ifndef KDEV_TESTMIPARSER_H
#define KDEV_TESTMIPARSER_H

#include <QObject>

namespace KDevMI { namespace MI { struct Value; }}


class TestMIParser : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testParseLine_data();
    void testParseLine();

private:
    void doTestResult(const KDevMI::MI::Value& actualValue, const QVariant& expectedValue);
};

#endif
