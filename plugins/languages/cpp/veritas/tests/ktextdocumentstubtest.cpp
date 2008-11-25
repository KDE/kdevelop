/*
* KDevelop xUnit testing support
* Copyright 2008 Manuel Breugelmans <mbr.nxi@gmail.com>
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU Library General Public License as
* published by the Free Software Foundation; either version 2 of the
* License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public
* License along with this program; if not, write to the
* Free Software Foundation, Inc.,
* 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/


#include "ktextdocumentstubtest.h"

#include "kdevtest.h"

#include "documentaccessstub.h"
#include <language/editor/simplerange.h>
#include <ktexteditor/range.h>
#include <ktexteditor/cursor.h>

using Veritas::Test::DocumentAccessStubTest;
using Veritas::Test::DocumentAccessStub;
using KTextEditor::Range;
using KTextEditor::Cursor;
using KDevelop::SimpleRange;

Q_DECLARE_METATYPE(KTextEditor::Range)
Q_DECLARE_METATYPE(KTextEditor::Cursor)

void DocumentAccessStubTest::init()
{
    qRegisterMetaType<Range>();
    qRegisterMetaType<Cursor>();
}

void DocumentAccessStubTest::cleanup()
{
}

void DocumentAccessStubTest::text()
{
    KUrl f;
    DocumentAccessStub* stub = new DocumentAccessStub;
    stub->m_text = QString("boo");
    KOMPARE("boo", stub->text(f));

    stub = new DocumentAccessStub();
    stub->m_text = "boo\nhoo";
    KOMPARE("boo\nhoo", stub->text(f));
}

void DocumentAccessStubTest::textRange_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<Range>("range");
    QTest::addColumn<QString>("expected");

    QString in("boo\nho\nzoop");
    QTest::newRow("01") << in << Range(0,0,0,1) << "b";
    QTest::newRow("02") << in << Range(0,1,0,2) << "o";
    QTest::newRow("03") << in << Range(0,0,0,3) << "boo";
    QTest::newRow("04") << in << Range(0,0,1,0) << "boo\n";
    QTest::newRow("05") << in << Range(1,0,1,2) << "ho";
    QTest::newRow("06") << in << Range(2,0,2,3) << "zoo";
    QTest::newRow("07") << in << Range(0,0,1,2) << "boo\nho";
    QTest::newRow("08") << in << Range(1,1,2,1) << "o\nz";
    QTest::newRow("09") << in << Range(0,0,2,4) << "boo\nho\nzoop";
    QTest::newRow("10") << in << Range(2,3,2,4) << "p";
    QTest::newRow("11") << in << Range(1,2,2,0) << "\n";
    QTest::newRow("12") << in << Range(0,0,4,0) << in + "\n"; // dont care about trailing newlines
}

void DocumentAccessStubTest::textRange()
{
    QFETCH(QString, expected);
    DocumentAccessStub* stub = new DocumentAccessStub;
    QFETCH(QString, input);
    stub->m_text = input;
    QFETCH(Range, range);
    KUrl url;
    SimpleRange sr(range);
    QString actual = stub->text(url,sr);
    KOMPARE(expected, actual); 
}

// void DocumentAccessStubTest::textRangeLines()
// {
//     DocumentAccessStub* stub = new DocumentAccessStub;
//     stub->m_text = "foo\nbar\nbaz\n";
//     Range range(1,0,1,3);
//     QStringList actual = stub->textLines(range);
//     KOMPARE(QStringList() << "bar", actual); 
// }
// 
// void DocumentAccessStubTest::character_data()
// {
//     QTest::addColumn<QString>("input");
//     QTest::addColumn<Cursor>("cursor");
//     QTest::addColumn<QChar>("expected");
// 
//     QString in("boo\nho\nzoap");
//     QTest::newRow("01") << in << Cursor(0,0) << QChar('b');
//     QTest::newRow("02") << in << Cursor(0,1) << QChar('o');
//     QTest::newRow("03") << in << Cursor(0,3) << QChar('\n');
//     QTest::newRow("04") << in << Cursor(1,0) << QChar('h');
//     QTest::newRow("05") << in << Cursor(2,2) << QChar('a');
//     QTest::newRow("06") << in << Cursor(2,3) << QChar('p');
// }
// 
// void DocumentAccessStubTest::character()
// {
//     DocumentAccessStub* stub = new DocumentAccessStub;
//     QFETCH(QString, input);
//     stub->m_text = input;
//     QFETCH(Cursor, cursor);
//     QChar actual = stub->character(cursor);
//     QFETCH(QChar, expected);
//     KOMPARE(expected, actual);
// }
// 
// void DocumentAccessStubTest::line_data()
// {
//     QTest::addColumn<QString>("input");
//     QTest::addColumn<int>("line");
//     QTest::addColumn<QString>("expected");
// 
//     QString in("boo\nho\nzoap\n\n");
//     QTest::newRow("01") << in << 0 << "boo\n";
//     QTest::newRow("02") << in << 1 << "ho\n";
//     QTest::newRow("03") << in << 2 << "zoap\n";
//     QTest::newRow("04") << in << 3 << "\n";
// }
// 
// void DocumentAccessStubTest::line()
// {
//     DocumentAccessStub* stub = new DocumentAccessStub;
//     QFETCH(QString, input);
//     stub->m_text = input;
//     QFETCH(int, line);
//     QString actual = stub->line(line);
//     QFETCH(QString, expected);
//     KOMPARE(expected, actual);
// }


QTEST_MAIN( DocumentAccessStubTest )
#include "ktextdocumentstubtest.moc"
