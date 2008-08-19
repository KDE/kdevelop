/* KDevelop xUnit plugin
 *
 * Copyright 2008 Manuel Breugelmans <mbr.nxi@gmail.com>
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

#ifndef QTEST_EXECUTABLE_STUB_H
#define QTEST_EXECUTABLE_STUB_H

#include "../executable.h"

namespace QTest { namespace Test {

class ExecutableStub : public QTest::Executable
{
public:
    ExecutableStub() : m_fetchFunctions(0) {}
    virtual ~ExecutableStub() {}
    virtual QStringList fetchFunctions() {
        return m_fetchFunctions;
    }
    QStringList m_fetchFunctions;
    virtual QString name() const {
        return m_name;
    }
    QString m_name;
    virtual KUrl location() const {
        return KUrl();
    }
};

}}

#endif // QTEST_EXECUTABLE_STUB_H
