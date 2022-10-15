/*
    SPDX-FileCopyrightText: 2016 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "test_stringhelpers.h"

#include <QTest>
#include <QStandardPaths>

#include <language/duchain/stringhelpers.h>

QTEST_MAIN(TestStringHelpers)

using namespace KDevelop;

void TestStringHelpers::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
}

void TestStringHelpers::testFormatComment_data()
{
    QTest::addColumn<QByteArray>("input");
    QTest::addColumn<QByteArray>("output");

    QTest::newRow("cpp-style") << QByteArrayLiteral(
        "// foo\n"
        "// bar"
    ) << QByteArrayLiteral("foo\n bar");

    QTest::newRow("doxy-cpp-style") << QByteArrayLiteral(
        "/// foo\n"
        "/// bar"
    ) << QByteArrayLiteral("foo\n bar");

    QTest::newRow("doxy-cpp-excl-style") << QByteArrayLiteral(
        "//! foo\n"
        "//! bar"
    ) << QByteArrayLiteral("foo\n bar");

    QTest::newRow("doxy-cpp-singleline-style") << QByteArrayLiteral("///< foo") << QByteArrayLiteral("foo");
    QTest::newRow("doxy-cpp-excl-singleline-style") << QByteArrayLiteral("//!< foo") << QByteArrayLiteral("foo");

    QTest::newRow("c-style") << QByteArrayLiteral(
        "/*\n"
        " foo\n"
        " bar\n*/"
    ) << QByteArrayLiteral("foo\nbar");

    QTest::newRow("doxy-c-style") << QByteArrayLiteral(
        "/**\n"
        " * foo\n"
        " * bar\n */"
    ) << QByteArrayLiteral("foo\n bar");

    QTest::newRow("doxy-c-style2") << QByteArrayLiteral(
        "/**\n"
        " * foo\n"
        " * bar\n **/"
    ) << QByteArrayLiteral("foo\n bar");

    QTest::newRow("real multiline") << QByteArrayLiteral(
                          "/**\n"
                          " * This is a real comment of some imaginary code.\n"
                          " *\n"
                          " * @param foo bar\n"
                          " * @return meh\n"
                          " */\n"
                      )
                    << QByteArrayLiteral("This is a real comment of some imaginary code.\n\n @param foo bar\n @return meh");
    QTest::newRow("doxy-qt-style-after-member") << QByteArrayLiteral(
        "/*!< line1\n"
        "line2 */"
    ) << QByteArrayLiteral("line1\nline2");

    QTest::newRow("doxy-c-style-after-member") << QByteArrayLiteral(
        "/**< line1\n"
        "line2 */"
    ) << QByteArrayLiteral("line1\nline2");

    QTest::newRow("doxy-cpp-style-after-member") << QByteArrayLiteral(
        "//!< line1\n"
        "//!< line2"
    ) << QByteArrayLiteral("line1\n line2");

    QTest::newRow("doxy-cpp-style-after-member2") << QByteArrayLiteral(
        "/// line1\n"
        "/// < line2"
    ) << QByteArrayLiteral("line1\n < line2");

    QTest::newRow("doxy-qt-style-before-member") << QByteArrayLiteral(
        "/*! line1\n"
        "line2 */"
    ) << QByteArrayLiteral("line1\nline2");

    QTest::newRow("doxy-qt-style-before-member2") << QByteArrayLiteral(
        "/*! line1\n"
        " * *line2* */"
    ) << QByteArrayLiteral("line1\n *line2*");

    QTest::newRow("doxy-cpp-style-before-member") << QByteArrayLiteral(
        "//! line1\n"
        "//! line2"
    ) << QByteArrayLiteral("line1\n line2");
}

void TestStringHelpers::testFormatComment()
{
    QFETCH(QByteArray, input);
    QFETCH(QByteArray, output);

    QCOMPARE(formatComment(input), output);
}

void TestStringHelpers::benchFormatComment()
{
    QBENCHMARK {
        formatComment(QByteArrayLiteral(
                          "/**\n"
                          " * This is a real comment of some imaginary code.\n"
                          " *\n"
                          " * @param foo bar\n"
                          " * @return meh\n"
                          " */\n"
                      ));
    }
}

void TestStringHelpers::testParamIterator_data()
{
    QTest::addColumn<QString>("parens");
    QTest::addColumn<QString>("source");
    QTest::addColumn<QStringList>("params");

    auto addTest = [](const QString& source, const QStringList& params) {
        QTest::addRow("%s", qPrintable(source)) << QStringLiteral("<>:") << source << params;
    };
    auto addMacroTest = [](const QString& source, const QStringList& params) {
        QTest::addRow("%s", qPrintable(source)) << QStringLiteral("()") << source << params;
    };

    addTest("Empty", {});
    addTest("Foo<T1, T2>", {"T1", "T2"});
    addTest("operator<", {});

    // These are valid C++ snippets, but I couldn't make such strings be passed to ParamIterator in practice.
    addTest("operator< <QString>", {"QString"});
    addTest("operator<<<KDevVarLengthArray, Path >", {"KDevVarLengthArray", "Path"});
    addTest("operator<=>< QRegularExpression,IndexedString*,char\t>", {"QRegularExpression", "IndexedString*", "char"});

    addTest("__not_overloaded2<_Tp, foo<bar>, __void_t<decltype(std::declval<_Tp>().operator<(std::declval<_Up>()))>>",
            {"_Tp", "foo<bar>", "__void_t<decltype(std::declval<_Tp>().operator<(std::declval<_Up>()))>"});
    addTest("__not_overloaded2<_Tp, foo<bar>, __void_t<decltype(std::declval<_Tp>().operator>(std::declval<_Up>()))>>",
            {"_Tp", "foo<bar>", "__void_t<decltype(std::declval<_Tp>().operator>(std::declval<_Up>()))>"});
    addTest("__not_overloaded2<_Tp, foo<bar>, __void_t<decltype(std::declval<_Tp>().operator>=(std::declval<_Up>()))>>",
            {"_Tp", "foo<bar>", "__void_t<decltype(std::declval<_Tp>().operator>=(std::declval<_Up>()))>"});
    addTest("__not_overloaded2<_Tp, foo<bar>, __void_t<decltype(std::declval<_Tp>().operator<=(std::declval<_Up>()))>>",
            {"_Tp", "foo<bar>", "__void_t<decltype(std::declval<_Tp>().operator<=(std::declval<_Up>()))>"});
    addTest("__not_overloaded2<_Tp, foo<bar>, __void_t<decltype(std::declval<_Tp>().operator>>(std::declval<_Up>()))>>",
            {"_Tp", "foo<bar>", "__void_t<decltype(std::declval<_Tp>().operator>>(std::declval<_Up>()))>"});
    addTest("__not_overloaded2<_Tp, foo<bar>, __void_t<decltype(std::declval<_Tp>().operator<<(std::declval<_Up>()))>>",
            {"_Tp", "foo<bar>", "__void_t<decltype(std::declval<_Tp>().operator<<(std::declval<_Up>()))>"});
    addTest("__not_overloaded2<_Tp, foo<bar>, __void_t<decltype(std::declval<_Tp>().operator<=>(std::declval<_Up>()))>>",
            {"_Tp", "foo<bar>", "__void_t<decltype(std::declval<_Tp>().operator<=>(std::declval<_Up>()))>"});
    addTest("__not_overloaded2<_Tp, foo<bar>, __void_t<decltype(std::declval<_Tp>().operator->(std::declval<_Up>()))>>",
            {"_Tp", "foo<bar>", "__void_t<decltype(std::declval<_Tp>().operator->(std::declval<_Up>()))>"});
    addTest("A<\">\\\">\">", {"\">\\\">\""});
    addTest("A<'>'>", {"'>'"});
    addTest("myoperator<anoperator<anotheroperator>, my_operator>", {"anoperator<anotheroperator>", "my_operator"});
    // c++17 operator<=
    addTest("Y<decltype(&X::operator<=), &X::operator<=>", {"decltype(&X::operator<=)", "&X::operator<="});
    // c++20 operator<=>
    addTest("Y<decltype(&X::operator<=>), &X::operator<=>>", {"decltype(&X::operator<=>)", "&X::operator<=>"});
    addTest("Y<decltype(&X::operator->), &X::operator->>", {"decltype(&X::operator->)", "&X::operator->"});
    addTest("Y<decltype(&X::operator->), Z<&X::operator->>>", {"decltype(&X::operator->)", "Z<&X::operator->>"});
    addTest("Y<decltype(&X::operator--), &X::operator-->", {"decltype(&X::operator--)", "&X::operator--"});
    addTest("Y<decltype(&X::operator--), Z<&X::operator-->>", {"decltype(&X::operator--)", "Z<&X::operator-->"});
    // c++17 operator<=
    addTest("Y<decltype(&X::operator<=), Z<&X::operator<=>>", {"decltype(&X::operator<=)", "Z<&X::operator<=>"});
    // c++20 operator<=>
    addTest("Y<decltype(&X::operator<=>), Z<&X::operator<=>>>", {"decltype(&X::operator<=>)", "Z<&X::operator<=>>"});
    // NOTE: this identifier here is invalid but we shouldn't trigger UB either, so the test is just that we get _something_ (even if it's wrong)
    addTest("bogus<_Tp, _Up, invalid<decltype(<=(std::declval<_Tp>(), std::declval<_Up>()))>>", {"_Tp", "_Up", "invalid<decltype(<=(std::declval<_Tp>(), std::declval<_Up>()))>>"});
    addTest("hardToParse<A<B>", {"A<B"});
    addTest("hardToParse<(A>B)>", {"(A>B)"});

    // Such zero/empty tparam strings are actually passed to ParamIterator() while libstdc++ headers are parsed.
    addTest("_Index_tuple<>", {});
    addTest("const __shared_count<  \t>", {});
    addTest("_Rb_tree_impl<_Key_compare, >", {"_Key_compare", ""});
    addTest("__hash_enum<_Tp,\t  \t>", {"_Tp", ""});
    addTest("__uniq_ptr_data<_Tp, _Dp, , >", {"_Tp", "_Dp", "", ""});

    addMacroTest("Q_UNIMPLEMENTED() qWarning(\"Unimplemented code.\")", {});
    addMacroTest("Q_FALLTHROUGH( ) [[clang::fallthrough]]", {});
    addMacroTest("( /*a)b*/ x , /*,*/y,z )", {"/*a)b*/ x", "/*,*/y", "z"});
}

void TestStringHelpers::testParamIterator()
{
    QFETCH(QString, parens);
    QFETCH(QString, source);
    QFETCH(QStringList, params);

    auto it = KDevelop::ParamIterator(parens, source);

    QEXPECT_FAIL("hardToParse<A<B>", "quasi impossible to parse without semantic knowledge of the types", Abort);

    int i = 0;
    while (!params.isEmpty()) {
        QVERIFY(it);
        if (i == 1) {
            QEXPECT_FAIL("Y<decltype(&X::operator<=), &X::operator<=>", "clang triggers warning for this C++17 code, due to C++20 spaceship op", Continue);
            QEXPECT_FAIL("Y<decltype(&X::operator<=), Z<&X::operator<=>>",
                         "clang triggers warning for this C++17 code, due to C++20 spaceship op", Continue);
        }
        QCOMPARE(*it, params.takeFirst());
        ++it;
        ++i;
    }

    QVERIFY(!it);
}
