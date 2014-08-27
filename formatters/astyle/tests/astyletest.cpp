/*
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "astyletest.h"

#include <QtTest/QTest>
#include <QDebug>

#include "../astyle_formatter.h"
#include <util/formattinghelpers.h>

QTEST_MAIN(AstyleTest)

void AstyleTest::initTestCase()
{
    m_formatter = new AStyleFormatter;
    ///TODO: probably all settings should be covered by tests
    ///      or at least set so we can be sure about what we
    ///      actually test...
    m_formatter->setSpaceIndentation(4);
}

void AstyleTest::renameVariable()
{
    // think this:
    // int asdf = 1;
    // e.g. asdf was before something different and got renamed
    QString formattedSource = m_formatter->formatSource(
        "asdf", "int ", " = 1;"
    );
    qDebug() << "formatted source:" << formattedSource;
    QCOMPARE(formattedSource, QString("asdf"));

    // int main() {
    //     if(asdf){}}
    formattedSource = m_formatter->formatSource(
        "asdf", "int main(){\n     if(", "){}}"
    );
    qDebug() << "formatted source:" << formattedSource;
    QCOMPARE(formattedSource, QString("asdf"));
}

void AstyleTest::testFuzzyMatching()
{
    // Some formatting styles inserts "{" and "}" parens behind "ifs", or change comment styles
    // The actual text changes, thus it is difficult to match original and formatted text

    QString leftContext = "void b() {/*some comment*/\nif( ";
    QString center = "a[   0]";
    QString rightContext =  " ) q;\n }\n";
    QString text = leftContext + center + rightContext;
    QString formatted = "void b() {// some comment\n    if( a[0] ) {\n        q;\n    }\n }\n";
    QString extracted = KDevelop::extractFormattedTextFromContext( formatted, text, QString(), QString() );
    QCOMPARE( extracted, formatted );
    
    extracted = KDevelop::extractFormattedTextFromContext( formatted, center, leftContext, rightContext );
    qDebug() << "extracted" << extracted << "formatted" << formatted;
    QCOMPARE( extracted, QString("a[0]") );
}

void AstyleTest::testTabMatching()
{
    // Some formatting styles inserts "{" and "}" parens behind "ifs", or change comment styles
    // The actual text changes, thus it is difficult to match original and formatted text

{
	// Mismatch: There is a preceding tab, but the formatter replaces the tab with spaces
	// The tab is matched with 2 spaces, since we set tab-width 2
    QString extracted = KDevelop::extractFormattedTextFromContext(
		"class C {\n  class A;\n}\n",
		"class A;", "class C {\n	", "\n}\n", 2 );
    QCOMPARE( extracted, QString("class A;") );
	
	// Two tabs are inserted insead of 1
	extracted = KDevelop::extractFormattedTextFromContext(
		"class C {\n		class A;\n}\n",
		"class A;", "class C {\n	", "\n}\n", 2 );
    QCOMPARE( extracted, QString("	class A;") );
	
	// One space is inserted behind the tab
	extracted = KDevelop::extractFormattedTextFromContext(
		"class C {\n	 class A;\n}\n",
		"class A;", "class C {\n	", "\n}\n", 2 );
    QCOMPARE( extracted, QString(" class A;") );

	// Two tabs are inserted, with 2 preceding whitespaces
	// Add only 1 tab
	extracted = KDevelop::extractFormattedTextFromContext(
		"class C {\n		class A;\n}\n",
		"class A;", "class C {\n  ", "\n}\n", 2 );
    QCOMPARE( extracted, QString("	class A;") );

	extracted = KDevelop::extractFormattedTextFromContext(
		"class C {\n          class A;\n}\n",
		"class A;", "class C {\n		", "\n}\n", 4 );
    QCOMPARE( extracted, QString("  class A;") );
}
{
	// Already correctly formatted
    QString leftContext = "void b() {\n c = 4;\n	";
    QString center = "a = 3;";
    QString rightContext =  "\n b = 5;\n }\n";
    QString text = leftContext + center + rightContext;
    QString formatted = "void b() {\n	c = 4;\n	a = 3;\n	b = 5;\n }\n";
    QString extracted = KDevelop::extractFormattedTextFromContext( formatted, text, QString(), QString() );
    QCOMPARE( extracted, formatted );
    
    extracted = KDevelop::extractFormattedTextFromContext( formatted, center, leftContext, rightContext );
    QCOMPARE( extracted, QString("a = 3;") );
}
{
    QString leftContext = "void b() {\n c = 4;\n";
    QString center = "a = 3;\n";
    QString rightContext =  "b = 5;\n }\n";
    QString text = leftContext + center + rightContext;
    QString formatted = "void b() {\n	c = 4;\n	a = 3;\n	b = 5;\n }\n";
    QString extracted = KDevelop::extractFormattedTextFromContext( formatted, text, QString(), QString() );
    QCOMPARE( extracted, formatted );
    
    extracted = KDevelop::extractFormattedTextFromContext( formatted, center, leftContext, rightContext );
    QCOMPARE( extracted, QString("	a = 3;\n	") );
}
}

void AstyleTest::overrideHelper()
{
    // think this:
    // virtual void asdf();
    // gets included into a class

    // test1: not indented
    QString formattedSource = m_formatter->formatSource(
        "virtual void asdf();", "class asdf {\n    int bar();\n", "\n};"
    );
    qDebug() << "formatted source:" << formattedSource;
    QCOMPARE(formattedSource, QString("    virtual void asdf();"));

    // test2: already indented
    formattedSource = m_formatter->formatSource(
        "virtual void asdf();", "class asdf {\n    int bar();\n    ", "\n};"
    );
    qDebug() << "formatted source:" << formattedSource;
    QCOMPARE(formattedSource, QString("virtual void asdf();"));
}

void AstyleTest::varTypeAssistant()
{
    // think this:
    // asdf = 1;
    // and you execute the assitant to get:
    // int asdf = 1;

    // test1: already indented
    QString formattedSource = m_formatter->formatSource(
        "int ", "int main() {\n    ", "asdf = 1;\n}\n"
    );
    qDebug() << "formatted source:" << formattedSource;
    QCOMPARE(formattedSource, QString("int "));

    // test2: not yet indented
    formattedSource = m_formatter->formatSource(
        "int ", "int main() {\n", "asdf = 1;\n}\n"
    );
    qDebug() << "formatted source:" << formattedSource;
    QCOMPARE(formattedSource, QString("    int "));

}

void AstyleTest::testMultipleFormatters()
{
    // just test that multiple formatters can exist at the same time
    AStyleFormatter* formatter1 = new AStyleFormatter;
    AStyleFormatter* formatter2 = new AStyleFormatter;
    delete formatter1;
    delete formatter2;
}

void AstyleTest::testMacroFormatting()
{
    AStyleFormatter fmt;
    fmt.setSpaceIndentation(2);
    fmt.setPreprocessorIndent(true);
    QString formatted = fmt.formatSource("#define asdf\\\nfoobar\n");
    QCOMPARE(formatted, QString("#define asdf\\\n  foobar\n"));
}

void AstyleTest::testContext()
{
    AStyleFormatter* formatter = new AStyleFormatter;
    formatter->setBracketFormatMode(astyle::LINUX_MODE);
    formatter->setParensInsidePaddingMode(true);
    formatter->setBlockIndent(true);
    // We enable break-blocks mode, so that we can test the newline matching
    formatter->setBreakBlocksMode(true);
    formatter->setBreakClosingHeaderBlocksMode(true);
    formatter->setParensUnPaddingMode(true);
    
    QString leftContext = "int main() {\n";
    QString rightContext = ";\n}\n";
    
    /// Newline tests
    
    QString formattedSource = formatter->formatSource(
        " int a;\n", leftContext, "int b;" + rightContext );
    
//     qDebug() << formattedSource;
    // Adjust indentation
    QCOMPARE(formattedSource, QString("    int a;\n    "));
    
    formattedSource = formatter->formatSource(
        " int a;\n", leftContext + " ", "   int b;" + rightContext );
    
//     qDebug() << formattedSource;
    QCOMPARE(formattedSource, QString("   int a;\n "));
    
    /// "if(a);" is interpreted as own block, so due to the "break blocks" option,
    /// astyle breaks these blocks with a newline in between.
    formattedSource = formatter->formatSource(
        "  if(a); ", leftContext + " if(a); ", " if(a);" + rightContext );
    
//     qDebug() << formattedSource;
    QCOMPARE(formattedSource, QString("\n\n    if( a );\n\n   "));

    formattedSource = formatter->formatSource(
        "  if(a); ", leftContext + " if(a);\n", " \n if(a);" + rightContext );
    
//     qDebug() << formattedSource;
    QCOMPARE(formattedSource, QString("\n    if( a );\n"));

    formattedSource = formatter->formatSource(
        "  if(a)\na; ", leftContext + " if(a);\n", " \n\n if(a);" + rightContext );
    
//     qDebug() << formattedSource;
    // Adjust indentation, successor already partially indentend
    QCOMPARE(formattedSource, QString("\n    if( a )\n        a;"));
    
    /// Whitespace tests
    
    formattedSource = formatter->formatSource(
        "int ", leftContext + "  ", rightContext );
    
    // 2 whitespaces are already in the context, so add only 2
//     qDebug() << "formatted source:" << formattedSource;
    QCOMPARE(formattedSource, QString("  int "));

    formattedSource = formatter->formatSource(
        "q", leftContext + "  if(", ")" + rightContext );
    
    // Padding was added around both parens
    QCOMPARE(formattedSource, QString(" q "));

    formattedSource = formatter->formatSource(
        "q", leftContext + "  if( ", " )" + rightContext );
    
    // Padding already existed around both parens
    QCOMPARE(formattedSource, QString("q"));

    formattedSource = formatter->formatSource(
        " q ", leftContext + "  if(", "   )" + rightContext );
    
//     qDebug() << formattedSource;
    // No padding on left, too much padding on right
    QCOMPARE(formattedSource, QString(" q"));

    formattedSource = formatter->formatSource(
        "   ", leftContext + "  if(q", ")" + rightContext );
    
//     qDebug() << formattedSource;
    // Normalize padding: from 3 to 1
    QCOMPARE(formattedSource, QString(" "));
    
    formattedSource = formatter->formatSource(
        "", leftContext + "  if(", "q )" + rightContext );
    
//     qDebug() << formattedSource;
    // Normalize padding: from 0 to 1
    QCOMPARE(formattedSource, QString(" "));
    
    formattedSource = formatter->formatSource(
        " ", leftContext + "  if(   ", "q )" + rightContext );
    
//     qDebug() << formattedSource;
    // Reduce padding as much as possible
    QCOMPARE(formattedSource, QString(""));
    
    delete formatter;
}

void AstyleTest::testTabIndentation()
{
    AStyleFormatter formatter;
    formatter.setTabSpaceConversionMode(false);
    formatter.setTabIndentation(2, false);

    const QString initial("int a() {\n  return 0;\n}\n");
    const QString expected("int a() {\n\treturn 0;\n}\n");
    const QString formatted = formatter.formatSource(initial);
    QCOMPARE(formatted, expected);
}

void AstyleTest::testForeach()
{
    AStyleFormatter formatter;
    QVERIFY(formatter.predefinedStyle("KDELibs"));

    const QString initial("int a(){QList<int> v;foreach(int i,v){return i;}}\n");
    const QString expected("int a()\n{\n    QList<int> v;\n    foreach (int i, v) {\n        return i;\n    }\n}\n");
    const QString formatted = formatter.formatSource(initial);
    QCOMPARE(formatted, expected);
}

