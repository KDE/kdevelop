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


#include "includewritertest.h"
#include "kdevtest.h"
#include "../includewriter.h"

using Veritas::Test::IncludeWriterTest;
using Veritas::IncludeSerializer;
using Veritas::IncludeGuardSerializer;

QString IncludeWriterTest::construct(const QString included, const QString includer)
{
    QBuffer buff;
    IncludeSerializer is;
    is.write(included, includer, &buff);
    return QString(buff.data());
}

void IncludeWriterTest::sameDir()
{
    QString included("/path/to/foo.h");
    QString includer("/path/to/bar.h");

    QString actual = construct(included, includer);
    QString expected = "#include \"foo.h\"";

    assertAlike(expected, actual);
}

void IncludeWriterTest::oneDirUp()
{
    QString included("/path/to/foo.h");
    QString includer("/path/to/more/bar.h");

    QString actual = construct(included, includer);
    QString expected = "#include \"../foo.h\"";

    assertAlike(expected, actual);
}

void IncludeWriterTest::oneDirDown()
{
    QString included("/path/to/more/foo.h");
    QString includer("/path/to/bar.h");

    QString actual = construct(included, includer);
    QString expected = "#include \"more/foo.h\"";

    assertAlike(expected, actual);
}

void IncludeWriterTest::guards()
{
    QBuffer buff;
    IncludeGuardSerializer igs;
    QString filename("/path/to/foo.h");

    igs.writeOpen(filename, &buff);
    QString actual = QString(buff.data());
    QString expected("#ifndef PATH_TO_FOO_H_INCLUDED\n#define PATH_TO_FOO_H_INCLUDED");

    assertAlike(expected, actual);

    QBuffer buff2;
    igs.writeClose(filename, &buff2);
    actual = QString(buff2.data());
    expected = QString("#endif // PATH_TO_FOO_H_INCLUDED");

    assertAlike(expected, actual);
}

////////////////////// Helpers ///////////////////////////////////////////////

/*! Construct a regular expression that is tolerant on
spaces, newlines tabs etc */
QRegExp IncludeWriterTest::whiteSpaceRegex(const QString& text)
{
  QString ws("[\\s\\n\\r\\t]*");
  QString escaped = QRegExp::escape(text);
  return QRegExp(ws + escaped + ws);
}

// custom assertion
void IncludeWriterTest::assertAlike(const QString& expected, const QString& actual)
{
  QRegExp pattern = whiteSpaceRegex(expected);
  KVERIFY_MSG(pattern.isValid(), pattern.errorString() + " " +pattern.pattern());
  QString failMsg = QString("\nExpected:\n%1\nActual:\n%2").arg(expected).arg(actual);
  KVERIFY_MSG(pattern.exactMatch(actual), failMsg);
}

QTEST_MAIN( IncludeWriterTest )
#include "includewritertest.moc"
