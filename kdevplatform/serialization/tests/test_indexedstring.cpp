/*
    SPDX-FileCopyrightText: 2012-2013 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "test_indexedstring.h"

#include <serialization/indexedstring.h>
#include <serialization/referencecounting.h>

#include <QTest>

#include <cstddef>
#include <utility>

QTEST_GUILESS_MAIN(TestIndexedString)

using namespace KDevelop;

void TestIndexedString::testUrl_data()
{
    QTest::addColumn<QUrl>("url");
    QTest::addColumn<QString>("string");
    QTest::newRow("empty") << QUrl() << QString();
    QTest::newRow("/") << QUrl::fromLocalFile(QStringLiteral("/")) << QStringLiteral("/");
    QTest::newRow("/foo/bar") << QUrl::fromLocalFile(QStringLiteral("/foo/bar")) << QStringLiteral("/foo/bar");
    QTest::newRow("http://foo.com/") << QUrl(QStringLiteral("http://foo.com/")) << QStringLiteral("http://foo.com/");
    QTest::newRow("http://foo.com/bar/asdf") << QUrl(QStringLiteral("http://foo.com/bar/asdf")) << QStringLiteral(
        "http://foo.com/bar/asdf");
    QTest::newRow("file:///bar/asdf") << QUrl(QStringLiteral("file:///bar/asdf")) << QStringLiteral("/bar/asdf");

#ifdef Q_OS_WIN
    // Make sure we're not running into https://bugreports.qt.io/browse/QTBUG-41729
    QTest::newRow("file:///C:/bar/asdf") << QUrl("file:///C:/bar/asdf") << QStringLiteral("C:/bar/asdf");
#endif
}

void TestIndexedString::testUrl()
{
    QFETCH(QUrl, url);
    IndexedString indexed(url);
    QCOMPARE(indexed.toUrl(), url);
    QTEST(indexed.str(), "string");
}

void TestIndexedString::test()
{
    QFETCH(QString, data);

    IndexedString indexed(data);
    QCOMPARE(indexed.str(), data);
    QCOMPARE(indexed.index(), IndexedString::indexForString(data));
    const auto byteArrayData = data.toUtf8();
    QEXPECT_FAIL("char-utf8", "UTF-8 gets decoded and the char data is stored internally", Continue);
    QEXPECT_FAIL("string-utf8", "UTF-8 gets decoded and the char data is stored internally", Continue);
    QCOMPARE(indexed.length(), data.length());
    // fallback until we rely on internal utf8 encoding
    QCOMPARE(indexed.length(), byteArrayData.length());

    QCOMPARE(indexed.byteArray(), byteArrayData);
    QVERIFY(!strncmp(indexed.c_str(), byteArrayData.data(), byteArrayData.length()));
    QCOMPARE(indexed.index(), IndexedString::indexForString(byteArrayData.data(), byteArrayData.length()));

    IndexedString moved = std::move(indexed);
    QCOMPARE(moved.str(), data);
}

void TestIndexedString::test_data()
{
    QTest::addColumn<QString>("data");

    QTest::newRow("empty") << QString();
    QTest::newRow("char-ascii") << QStringLiteral("a");
    QTest::newRow("char-utf8") << QStringLiteral("ä");
    QTest::newRow("string-ascii") << QStringLiteral("asdf()?=");
    QTest::newRow("string-utf8") << QStringLiteral("æſðđäöü");
}

void TestIndexedString::testMoveAssignment()
{
    // Deliberately make no assumptions about the value of the moved-from object.

    const QString text = QStringLiteral("random text");
    IndexedString movedFrom(text);
    QCOMPARE(movedFrom.str(), text);

    IndexedString notRefCounted;
    notRefCounted = std::move(movedFrom);
    QCOMPARE(notRefCounted.str(), text);

    movedFrom = std::move(notRefCounted);
    QCOMPARE(movedFrom.str(), text);

    const QString toBeDerefed = QStringLiteral("to-be-derefed");

    // Enable reference counting throughout refCounted's lifetime.
    std::byte indexedStringData[sizeof(IndexedString)];
    const DUChainReferenceCountingEnabler rcEnabler(indexedStringData, sizeof(IndexedString));
    IndexedString* const refCounted = new (indexedStringData) IndexedString(toBeDerefed);
    QCOMPARE(refCounted->str(), toBeDerefed);

    *refCounted = std::move(movedFrom);
    QCOMPARE(refCounted->str(), text);
    refCounted->~IndexedString();
}

void TestIndexedString::testSwap()
{
    QFETCH(bool, lhsRcEnabled);
    QFETCH(bool, rhsRcEnabled);

    class OptionallyRcString
    {
    public:
        explicit OptionallyRcString(bool rcEnabled, const QString& initText)
            : m_rcEnabled{rcEnabled}
            , m_initText{initText}
            , m_notRefCounted{m_initText}
            , m_refCountedData{}
            , m_rcEnabler{m_refCountedData, sizeof(IndexedString)}
            , m_string{m_rcEnabled ? new (m_refCountedData) IndexedString(m_initText) : &m_notRefCounted}
        {
            QCOMPARE(m_string->str(), m_initText);
        }

        ~OptionallyRcString()
        {
            if (m_rcEnabled) {
                m_string->~IndexedString();
            }
        }

        const QString& initText() const { return m_initText; }
        IndexedString& string() { return *m_string; }

    private:
        const bool m_rcEnabled;
        const QString m_initText;

        IndexedString m_notRefCounted;
        std::byte m_refCountedData[sizeof(IndexedString)];
        const DUChainReferenceCountingEnabler m_rcEnabler;

        IndexedString* const m_string = nullptr;
    };

    OptionallyRcString lhs(lhsRcEnabled, QStringLiteral("1st text"));
    OptionallyRcString rhs(rhsRcEnabled, QStringLiteral("another string"));

    using std::swap;

    swap(lhs.string(), rhs.string());
    QCOMPARE(lhs.string().str(), rhs.initText());
    QCOMPARE(rhs.string().str(), lhs.initText());

    swap(lhs.string(), rhs.string());
    QCOMPARE(lhs.string().str(), lhs.initText());
    QCOMPARE(rhs.string().str(), rhs.initText());
}

void TestIndexedString::testSwap_data()
{
    QTest::addColumn<bool>("lhsRcEnabled");
    QTest::addColumn<bool>("rhsRcEnabled");

    QTest::newRow("no reference counting") << false << false;
    QTest::newRow("lhs reference-counted") << true << false;
    QTest::newRow("rhs reference-counted") << false << true;
    QTest::newRow("both reference-counted") << true << true;
}

void TestIndexedString::testCString()
{
    IndexedString str(nullptr);
    QCOMPARE(str.index(), 0u);
    QVERIFY(str.isEmpty());
}

#include "moc_test_indexedstring.cpp"
