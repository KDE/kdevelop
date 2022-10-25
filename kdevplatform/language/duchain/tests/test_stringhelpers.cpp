/*
    SPDX-FileCopyrightText: 2016 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "test_stringhelpers.h"

#include <QTest>
#include <QStandardPaths>

#include <language/duchain/stringhelpers.h>

#include <cstring>

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
    QTest::addColumn<int>("endPosition");

    auto addGeneralTest = [](const QString& parens, const QString& source, const QStringList& params,
                             int endPosition = -1) {
        QTest::addRow("%s", qPrintable(source))
            << parens << source << params << (endPosition == -1 ? source.size() : endPosition);
    };
    auto addTest = [addGeneralTest](const QString& source, const QStringList& params, int endPosition = -1) {
        addGeneralTest("<>:", source, params, endPosition);
    };
    auto addMacroTest = [addGeneralTest](const QString& source, const QStringList& params, int endPosition = -1) {
        addGeneralTest("()", source, params, endPosition);
    };

    addTest("Empty", {});
    addTest("Foo<T1, T2>", {"T1", "T2"});
    addTest("operator<", {});

    // Such strings are passed to ParamIterator() when an Identifier is constructed from a template argument inside the
    // call to Visitor::makeType() in Visitor::createClassTemplateSpecializationType(). For example, the following valid
    // C++ code `K< &operator<< <S> > k;` passes the string "&operator<<<S>" to ParamIterator().
    addTest("operator< <QString>", {"QString"});
    addTest("operator<<<KDevVarLengthArray, Path >", {"KDevVarLengthArray", "Path"});
    addTest("operator<=>< QRegularExpression,IndexedString*,char\t>", {"QRegularExpression", "IndexedString*", "char"});
    // The valid C++ code `C<decltype(A::f<int>)> c;` passes the string "decltype(A::f<int>)" to ParamIterator().
    addTest("decltype(f<int>)", {});
    addTest("decltype(A::f)", {});
    addTest("decltype(A::f<int>)", {});
    addTest("decltype(N::operator<  <N::String>)", {});

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
    // NOTE: the "bogus" identifiers below are invalid but shouldn't trigger UB, so the tests verify that we get
    // _something_ (even if it's wrong).
    addTest("bogus<_Tp, _Up, invalid<decltype(<=(std::declval<_Tp>(), std::declval<_Up>()))>>",
            {"_Tp", "_Up", "invalid<decltype(<=(std::declval<_Tp>(), std::declval<_Up>()))>"});
    addTest("bogus<_Tp, _Up, invalid<<=(std::declval<_Tp>(), std::declval<_Up>())>>",
            {"_Tp", "_Up", "invalid<<=(std::declval<_Tp>(), std::declval<_Up>())>>"});
    addTest("hardToParse<A<B>", {"A<B"});
    addTest("hardToParse<(A<B)>", {"(A<B)"});
    addTest("hardToParse<(A>B)>", {"(A>B)"});
    addTest("hardToParse<Foo<(A<B)>>", {"Foo<(A<B)>"});
    addTest("hardToParse<Foo<(A>B)>>", {"Foo<(A>B)>"});

    // Such zero/empty tparam strings are actually passed to ParamIterator() while libstdc++ headers are parsed.
    addTest("_Index_tuple<>", {});
    addTest("const __shared_count<  \t>", {});
    addTest("_Rb_tree_impl<_Key_compare, >", {"_Key_compare", ""});
    addTest("__hash_enum<_Tp,\t  \t>", {"_Tp", ""});
    addTest("__uniq_ptr_data<_Tp, _Dp, , >", {"_Tp", "_Dp", "", ""});

    const auto addSpacedOperatorTest = [addTest](const QString& op) {
        const auto tArg1 = QStringLiteral("x %1 y").arg(op);
        const auto tArg2 = QStringLiteral("Foo<%1>").arg(tArg1);
        for (const auto& templateArgument : {tArg1, tArg2}) {
            addTest(QStringLiteral("spaced<%1>").arg(templateArgument), {templateArgument});
        }
    };

    addSpacedOperatorTest("<");
    addSpacedOperatorTest(">");
    addSpacedOperatorTest("<=");
    addSpacedOperatorTest(">=");
    addSpacedOperatorTest("<<");
    addSpacedOperatorTest(">>");
    addSpacedOperatorTest("<<=");
    addSpacedOperatorTest(">>=");
    addSpacedOperatorTest("<=>");

    addSpacedOperatorTest("->");
    // -> must be recognized even when not surrounded with spaces
    addTest("arrow<u->v>", {"u->v"});
    addTest("arrow<Foo<u->v>>", {"Foo<u->v>"});

    addTest("X::Y<Z>", {}, 1);
    addTest("X<Z::C>", {"Z::C"});

    addMacroTest("Q_UNIMPLEMENTED() qWarning(\"Unimplemented code.\")", {}, std::strlen("Q_UNIMPLEMENTED()"));
    addMacroTest("Q_FALLTHROUGH( ) [[clang::fallthrough]]", {}, std::strlen("Q_FALLTHROUGH( )"));
    addMacroTest("( /*a)b*/ x , /*,*/y,z )", {"/*a)b*/ x", "/*,*/y", "z"});
}

void TestStringHelpers::testParamIterator()
{
    QFETCH(QString, parens);
    QFETCH(QString, source);
    QFETCH(QStringList, params);
    QFETCH(int, endPosition);

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
    QCOMPARE(it.position(), endPosition);
}
