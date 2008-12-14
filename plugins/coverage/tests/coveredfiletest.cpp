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

#include "coveredfiletest.h"
#include "../coveredfile.h"

#include "testutils.h"

using Veritas::CoveredFile;
using Veritas::CoveredFileTest;

void CoveredFileTest::init()
{
    m_file = new CoveredFile;
}

void CoveredFileTest::cleanup()
{
    if (m_file) delete m_file;
}

void CoveredFileTest::construct()
{
    KOMPARE(KUrl(), m_file->url());
    KOMPARE(0, m_file->coverageRatio());
    KOMPARE(0, m_file->sloc());
    KOMPARE(0, m_file->nrofCoveredLines());
    KOMPARE(0, m_file->callCountMap().count());
}

void CoveredFileTest::sunny()
{
    m_file->setCallCount(1,0);
    m_file->setCallCount(2,0);
    m_file->setCallCount(3,1);
    m_file->setCallCount(4,1);
    KOMPARE(50.0, m_file->coverageRatio());
}

void CoveredFileTest::zeroSloc()
{
    KOMPARE(0, m_file->coverageRatio());
}

void CoveredFileTest::callCount()
{
    m_file->setCallCount(10, 5);
    m_file->setCallCount(11, 4);

    QMap<int, int> callMap = m_file->callCountMap();
    KOMPARE(2, callMap.count());
    KVERIFY(callMap.contains(10));
    KOMPARE(5, callMap[10]);
    KVERIFY(callMap.contains(11));
    KOMPARE(4, callMap[11]);
}

void CoveredFileTest::linesSet()
{
    m_file->setCallCount(9, 0);
    m_file->setCallCount(10, 5);
    m_file->setCallCount(11, 0);
    m_file->setCallCount(12, 3);


    QSet<int> lines;
    lines << 10 << 12;
    KOMPARE(lines, m_file->coveredLines());
}

void CoveredFileTest::isReachable()
{
    m_file->setCallCount(2,1);
    KVERIFY(m_file->isReachable(2));
    KVERIFY(!m_file->isReachable(1));

    m_file->setCallCount(1,0);
    KVERIFY(m_file->isReachable(1));
    KVERIFY(m_file->isReachable(2));
    KVERIFY(!m_file->isReachable(3));
}

QTEST_KDEMAIN( CoveredFileTest, NoGUI)
#include "coveredfiletest.moc"
