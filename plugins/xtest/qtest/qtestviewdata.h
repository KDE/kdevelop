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

#ifndef QTEST_QTESTVIEWDATA_H
#define QTEST_QTESTVIEWDATA_H

#include <veritas/itestrunner.h>
#include "qxqtestexport.h"

namespace Veritas { class Test; class ITestFramework; }
namespace QTest { class ISettings; }
class QTestOutputDelegate;

/*! Assorted qtest runner toolview data and operations */
class QXQTEST_EXPORT QTestViewData : public Veritas::ITestRunner
{
Q_OBJECT
public:
    QTestViewData(Veritas::ITestFramework* framework);
    virtual ~QTestViewData();
    void registerTests();
    QString fetchBuildRoot();
    QString fetchRegXML();

private slots:
    void resetLock();

private:
    QTest::ISettings* m_settings;
    bool m_lock;
};

#endif
