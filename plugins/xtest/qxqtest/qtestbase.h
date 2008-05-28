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

#ifndef QXQTEST_QTESTBASE_H
#define QXQTEST_QTESTBASE_H

#include <QString>
#include <QFileInfo>

namespace QxQTest
{

/**
 * Base for QTestCase, QTestCommand, QTestSuite
 **/
class QTestBase : public QObject
{
Q_OBJECT
public:
    QTestBase();
    QTestBase(const QString&, QTestBase* parent);
    virtual ~QTestBase();

    QString name();
    QTestBase* parent();

    void setName(const QString&);
    void setParent(QTestBase* parent);

private:
    QString m_name;
    QTestBase* m_parent;
};

} // end namespace QxQTest

#endif // QXQTEST_QTESTBASE_H
