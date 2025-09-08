/*
    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "test_astyle.h"

#include <QTest>
#include <QDebug>
#include <QStandardPaths>

#include "../astyle_formatter.h"
#include <util/formattinghelpers.h>

QTEST_MAIN(TestAstyle)

TestAstyle::~TestAstyle() = default;

void TestAstyle::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
    m_formatter = std::make_unique<AStyleFormatter>();
    ///TODO: probably all settings should be covered by tests
    ///      or at least set so we can be sure about what we
    ///      actually test...
    m_formatter->setSpaceIndentationNoConversion(4);
}

void TestAstyle::renameVariable()
{
    // think this:
    // int asdf = 1;
    // e.g. asdf was before something different and got renamed
    QString formattedSource = m_formatter->formatSource(
        QStringLiteral("asdf"), QStringLiteral("int "), QStringLiteral(" = 1;")
    );
    qDebug() << "formatted source:" << formattedSource;
    QCOMPARE(formattedSource, QString("asdf"));

    // int main() {
    //     if(asdf){}}
    formattedSource = m_formatter->formatSource(
        QStringLiteral("asdf"), QStringLiteral("int main(){\n     if("), QStringLiteral("){}}")
    );
    qDebug() << "formatted source:" << formattedSource;
    QCOMPARE(formattedSource, QString("asdf"));
}

void TestAstyle::testFuzzyMatching()
{
    // Some formatting styles inserts "{" and "}" parens behind "ifs", or change comment styles
    // The actual text changes, thus it is difficult to match original and formatted text

    QString leftContext = QStringLiteral("void b() {/*some comment*/\nif( ");
    QString center = QStringLiteral("a[   0]");
    QString rightContext =  QStringLiteral(" ) q;\n }\n");
    QString text = leftContext + center + rightContext;
    QString formatted = QStringLiteral("void b() {// some comment\n    if( a[0] ) {\n        q;\n    }\n }\n");
    QString extracted = KDevelop::extractFormattedTextFromContext( formatted, text, QString(), QString() );
    QCOMPARE( extracted, formatted );
    
    extracted = KDevelop::extractFormattedTextFromContext( formatted, center, leftContext, rightContext );
    qDebug() << "extracted" << extracted << "formatted" << formatted;
    QCOMPARE( extracted, QString("a[0]") );

    rightContext = QStringLiteral("\nvoid g() {}");
    extracted = KDevelop::extractFormattedTextFromContext(formatted + rightContext, text, QString(), rightContext);
    QCOMPARE(extracted, formatted);
}

void TestAstyle::testTabMatching()
{
    // Some formatting styles inserts "{" and "}" parens behind "ifs", or change comment styles
    // The actual text changes, thus it is difficult to match original and formatted text

{
	// Mismatch: There is a preceding tab, but the formatter replaces the tab with spaces
	// The tab is matched with 2 spaces, since we set tab-width 2
    QString extracted = KDevelop::extractFormattedTextFromContext(
		QStringLiteral("class C {\n  class A;\n}\n"),
		QStringLiteral("class A;"), QStringLiteral("class C {\n	"), QStringLiteral("\n}\n"), 2 );
    QCOMPARE( extracted, QString("class A;") );
	
	// Two tabs are inserted instead of 1
	extracted = KDevelop::extractFormattedTextFromContext(
		QStringLiteral("class C {\n		class A;\n}\n"),
		QStringLiteral("class A;"), QStringLiteral("class C {\n	"), QStringLiteral("\n}\n"), 2 );
    QCOMPARE( extracted, QString("	class A;") );
	
	// One space is inserted behind the tab
	extracted = KDevelop::extractFormattedTextFromContext(
		QStringLiteral("class C {\n	 class A;\n}\n"),
		QStringLiteral("class A;"), QStringLiteral("class C {\n	"), QStringLiteral("\n}\n"), 2 );
    QCOMPARE( extracted, QString(" class A;") );

	// Two tabs are inserted, with 2 preceding whitespaces
	// Add only 1 tab
	extracted = KDevelop::extractFormattedTextFromContext(
		QStringLiteral("class C {\n		class A;\n}\n"),
		QStringLiteral("class A;"), QStringLiteral("class C {\n  "), QStringLiteral("\n}\n"), 2 );
    QCOMPARE( extracted, QString("	class A;") );

	extracted = KDevelop::extractFormattedTextFromContext(
		QStringLiteral("class C {\n          class A;\n}\n"),
		QStringLiteral("class A;"), QStringLiteral("class C {\n		"), QStringLiteral("\n}\n"), 4 );
    QCOMPARE( extracted, QString("  class A;") );
}
{
	// Already correctly formatted
    QString leftContext = QStringLiteral("void b() {\n c = 4;\n	");
    QString center = QStringLiteral("a = 3;");
    QString rightContext =  QStringLiteral("\n b = 5;\n }\n");
    QString text = leftContext + center + rightContext;
    QString formatted = QStringLiteral("void b() {\n	c = 4;\n	a = 3;\n	b = 5;\n }\n");
    QString extracted = KDevelop::extractFormattedTextFromContext( formatted, text, QString(), QString() );
    QCOMPARE( extracted, formatted );
    
    extracted = KDevelop::extractFormattedTextFromContext( formatted, center, leftContext, rightContext );
    QCOMPARE( extracted, QString("a = 3;") );
}
{
    QString leftContext = QStringLiteral("void b() {\n c = 4;\n");
    QString center = QStringLiteral("a = 3;\n");
    QString rightContext =  QStringLiteral("b = 5;\n }\n");
    QString text = leftContext + center + rightContext;
    QString formatted = QStringLiteral("void b() {\n	c = 4;\n	a = 3;\n	b = 5;\n }\n");
    QString extracted = KDevelop::extractFormattedTextFromContext( formatted, text, QString(), QString() );
    QCOMPARE( extracted, formatted );
    
    extracted = KDevelop::extractFormattedTextFromContext( formatted, center, leftContext, rightContext );
    QCOMPARE( extracted, QString("	a = 3;\n	") );
}
}

void TestAstyle::overrideHelper()
{
    // think this:
    // virtual void asdf();
    // gets included into a class

    // test1: not indented
    QString formattedSource = m_formatter->formatSource(
        QStringLiteral("virtual void asdf();"), QStringLiteral("class asdf {\n    int bar();\n"), QStringLiteral("\n};")
    );
    qDebug() << "formatted source:" << formattedSource;
    QCOMPARE(formattedSource, QString("    virtual void asdf();"));

    // test2: already indented
    formattedSource = m_formatter->formatSource(
        QStringLiteral("virtual void asdf();"), QStringLiteral("class asdf {\n    int bar();\n    "), QStringLiteral("\n};")
    );
    qDebug() << "formatted source:" << formattedSource;
    QCOMPARE(formattedSource, QString("virtual void asdf();"));
}

void TestAstyle::varTypeAssistant()
{
    // think this:
    // asdf = 1;
    // and you execute the assistant to get:
    // int asdf = 1;

    // test1: already indented
    QString formattedSource = m_formatter->formatSource(
        QStringLiteral("int "), QStringLiteral("int main() {\n    "), QStringLiteral("asdf = 1;\n}\n")
    );
    qDebug() << "formatted source:" << formattedSource;
    QCOMPARE(formattedSource, QString("int "));

    // test2: not yet indented
    formattedSource = m_formatter->formatSource(
        QStringLiteral("int "), QStringLiteral("int main() {\n"), QStringLiteral("asdf = 1;\n}\n")
    );
    qDebug() << "formatted source:" << formattedSource;
    QCOMPARE(formattedSource, QString("    int "));

}

void TestAstyle::testMultipleFormatters()
{
    // just test that multiple formatters can exist at the same time
    auto* formatter1 = new AStyleFormatter;
    auto* formatter2 = new AStyleFormatter;
    delete formatter1;
    delete formatter2;
}

void TestAstyle::testMacroFormatting()
{
    AStyleFormatter fmt;
    fmt.setSpaceIndentationNoConversion(2);
    fmt.setPreprocessorIndent(true);
    QString formatted = fmt.formatSource(QStringLiteral("#define asdf\\\nfoobar\n"));
    QCOMPARE(formatted, QString("#define asdf\\\n  foobar\n"));
}

void TestAstyle::testContext()
{
    auto* formatter = new AStyleFormatter;
    formatter->setBracketFormatMode(astyle::LINUX_MODE);
    formatter->setParensInsidePaddingMode(true);
    formatter->setBlockIndent(true);
    // We enable break-blocks mode, so that we can test the newline matching
    formatter->setBreakBlocksMode(true);
    formatter->setBreakClosingHeaderBlocksMode(true);
    formatter->setParensUnPaddingMode(true);
    
    QString leftContext = QStringLiteral("int main() {\n");
    QString rightContext = QStringLiteral(";\n}\n");
    
    /// Newline tests
    
    QString formattedSource = formatter->formatSource(
        QStringLiteral(" int a;\n"), leftContext, "int b;" + rightContext );
    
//     qDebug() << formattedSource;
    // Adjust indentation
    QCOMPARE(formattedSource, QString("    int a;\n    "));
    
    formattedSource = formatter->formatSource(
        QStringLiteral(" int a;\n"), leftContext + " ", "   int b;" + rightContext );
    
//     qDebug() << formattedSource;
    QCOMPARE(formattedSource, QString("   int a;\n "));
    
    /// "if(a);" is interpreted as own block, so due to the "break blocks" option,
    /// astyle breaks these blocks with a newline in between.
    formattedSource = formatter->formatSource(
        QStringLiteral("  if(a); "), leftContext + " if(a); ", " if(a);" + rightContext );
    
//     qDebug() << formattedSource;
    QCOMPARE(formattedSource, QString("\n\n    if( a );\n\n   "));

    formattedSource = formatter->formatSource(
        QStringLiteral("  if(a); "), leftContext + " if(a);\n", " \n if(a);" + rightContext );
    
//     qDebug() << formattedSource;
    QCOMPARE(formattedSource, QString("\n    if( a );\n"));

    formattedSource = formatter->formatSource(
        QStringLiteral("  if(a)\na; "), leftContext + " if(a);\n", " \n\n if(a);" + rightContext );
    
//     qDebug() << formattedSource;
    // Adjust indentation, successor already partially indentend
    QCOMPARE(formattedSource, QString("\n    if( a )\n        a;"));
    
    /// Whitespace tests
    
    formattedSource = formatter->formatSource(
        QStringLiteral("int "), leftContext + "  ", rightContext );
    
    // 2 whitespaces are already in the context, so add only 2
//     qDebug() << "formatted source:" << formattedSource;
    QCOMPARE(formattedSource, QString("  int "));

    formattedSource = formatter->formatSource(
        QStringLiteral("q"), leftContext + "  if(", ")" + rightContext );
    
    // Padding was added around both parens
    QCOMPARE(formattedSource, QString(" q "));

    formattedSource = formatter->formatSource(
        QStringLiteral("q"), leftContext + "  if( ", " )" + rightContext );
    
    // Padding already existed around both parens
    QCOMPARE(formattedSource, QString("q"));

    formattedSource = formatter->formatSource(
        QStringLiteral(" q "), leftContext + "  if(", "   )" + rightContext );
    
//     qDebug() << formattedSource;
    // No padding on left, too much padding on right
    QCOMPARE(formattedSource, QString(" q"));

    formattedSource = formatter->formatSource(
        QStringLiteral("   "), leftContext + "  if(q", ")" + rightContext );
    
//     qDebug() << formattedSource;
    // Normalize padding: from 3 to 1
    QCOMPARE(formattedSource, QString(" "));
    
    formattedSource = formatter->formatSource(
        QString(), leftContext + "  if(", "q )" + rightContext );
    
//     qDebug() << formattedSource;
    // Normalize padding: from 0 to 1
    QCOMPARE(formattedSource, QString(" "));
    
    formattedSource = formatter->formatSource(
        QStringLiteral(" "), leftContext + "  if(   ", "q )" + rightContext );
    
//     qDebug() << formattedSource;
    // Reduce padding as much as possible
    QCOMPARE(formattedSource, QString());
    
    delete formatter;
}

void TestAstyle::testTabIndentation()
{
    AStyleFormatter formatter;
    formatter.setTabIndentation(2, false);

    const QString initial(QStringLiteral("int a() {\n  return 0;\n}\n"));
    const QString expected(QStringLiteral("int a() {\n\treturn 0;\n}\n"));
    const QString formatted = formatter.formatSource(initial);
    QCOMPARE(formatted, expected);
}

void TestAstyle::testForeach()
{
    AStyleFormatter formatter;
    QVERIFY(formatter.predefinedStyle("KDELibs"));

    const QString initial(QStringLiteral("int a(){QList<int> v;\n    foreach(int i,v){\nreturn i;}}\n"));
    const QString expected(QStringLiteral("int a()\n{\n    QList<int> v;\n    foreach (int i, v) {\n        return i;\n    }\n}\n"));
    const QString formatted = formatter.formatSource(initial);
    QCOMPARE(formatted, expected);
}

void TestAstyle::testPointerAlignment()
{
    AStyleFormatter formatter;
    formatter.setPointerAlignment(astyle::PTR_ALIGN_NAME);

    const QString initial(QStringLiteral("int* a;\nint * b;\nint *c;\nint & d;\nconst double * const e;\n"));
    const QString expected(QStringLiteral("int *a;\nint *b;\nint *c;\nint &d;\nconst double *const e;\n"));
    const QString formatted = formatter.formatSource(initial);
    QCOMPARE(formatted, expected);
}

void TestAstyle::testKdeFrameworks()
{
    AStyleFormatter formatter;
    QVERIFY(formatter.predefinedStyle("KDELibs"));

    QFETCH(QString, initial);
    QFETCH(QString, leftContext);
    QFETCH(QString, rightContext);
    QFETCH(QString, expected);

    const QString formatted = formatter.formatSource(initial, leftContext, rightContext);
    QCOMPARE(formatted, expected);
}

void TestAstyle::testKdeFrameworks_data()
{
    QTest::addColumn<QString>("initial");
    QTest::addColumn<QString>("leftContext");
    QTest::addColumn<QString>("rightContext");
    QTest::addColumn<QString>("expected");

    const QString leftContext = QStringLiteral("int main()\n{\n");
    const QString rightContext = QStringLiteral("\n}\n");

    QString initial = QStringLiteral("\t int a;");
    QString expected = QStringLiteral("    int a;");
    QTest::newRow("indentation") << initial << leftContext << rightContext << expected;

    initial = QStringLiteral("if(1);\n while (false);\nfor(int i=0; i<1; i++);\n");
    expected = QStringLiteral("    if (1);\n    while (false);\n    for (int i = 0; i < 1; i++);\n");
    QTest::newRow("space-after-keyword") << initial << leftContext << rightContext << expected;

    initial = QStringLiteral("int* a;\nint * b;\nint *c;\nint & d;\nconst double * const e;\n");
    expected = QStringLiteral("int *a;\nint *b;\nint *c;\nint &d;\nconst double *const e;\n");
    QTest::newRow("pointer-alignment") << initial << QString() << QString() << expected;

    initial = QStringLiteral("if (true)\n{\n}\n");
    expected = QStringLiteral("    if (true) {\n    }\n");
    QTest::newRow("brace-on-same-line") << initial << leftContext << rightContext << expected;

    initial = QStringLiteral("void foo(void) {\n}\nclass a {\n};\nnamespace c\n{\n}\n");
    expected = QStringLiteral("void foo(void)\n{\n}\nclass a\n{\n};\nnamespace c\n{\n}\n");
    QTest::newRow("brace-on-new-line") << initial << QString() << QString() << expected;

    initial = QStringLiteral("switch (myEnum)\n{\ncase Value1:\ndoSomething();}\n");
    expected = QStringLiteral("    switch (myEnum) {\n    case Value1:\n        doSomething();\n    }\n");
    QTest::newRow("switch-statement") << initial << leftContext << rightContext << expected;
}

#include "moc_test_astyle.cpp"
