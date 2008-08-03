#include "coveredfiletest.h"
#include "../coveredfile.h"
#include <QtTest/QTest>
#include <qtest_kde.h>
#include <veritas/tests/kasserts.h>

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
    KOMPARE(0, m_file->coverage());
    KOMPARE(0, m_file->sloc());
    KOMPARE(0, m_file->instrumented());
    KOMPARE(0, m_file->callCountMap().count());
}

void CoveredFileTest::sunny()
{
    m_file->setCallCount(1,0);
    m_file->setCallCount(2,0);
    m_file->setCallCount(3,1);
    m_file->setCallCount(4,1);
    KOMPARE(50.0, m_file->coverage());
}

void CoveredFileTest::zeroSloc()
{
    KOMPARE(0, m_file->coverage());
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

QTEST_KDEMAIN( CoveredFileTest, NoGUI )
#include "coveredfiletest.moc"
