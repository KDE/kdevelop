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

#ifndef QXQTEST_QTESTCASE_H
#define QXQTEST_QTESTCASE_H

#include <QString>
#include <QFileInfo>
#include <QList>

#include "qtestbase.h"
#include "qtestcommand.h"

namespace QxQTest
{

class QTestCase : public QTestBase
{
Q_OBJECT
public:
    QTestCase();
    QTestCase(const QString&, const QFileInfo&, QTestBase* parent);
    virtual ~QTestCase();

    void addTest(QTestCommand* test);
    QTestCommand* getTestAt(unsigned i);
    int nrofChildren();
    QFileInfo exe();

    void setExe(const QFileInfo&);

private:
    QFileInfo m_exe;
    QList<QTestCommand*> m_children;
};

} // end namespace QxQTest

#endif // QXQTEST_QTESTCASE_H
