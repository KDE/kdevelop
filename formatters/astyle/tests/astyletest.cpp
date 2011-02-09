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

#include "astyletest.moc"