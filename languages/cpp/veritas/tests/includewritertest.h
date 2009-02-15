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

#ifndef QTEST_INCLUDEWRITERTEST_H_INCLUDED
#define QTEST_INCLUDEWRITERTEST_H_INCLUDED

#include <QtCore/QObject>
#include <QString>
#include <QRegExp>

namespace Veritas { namespace Test {

/*! @unitundertest Veritas::IncludeSerializer */
class IncludeWriterTest : public QObject
{
Q_OBJECT
private slots:
    void sameDir();
    void oneDirUp();
    void oneDirDown();
    void guards();

private:
    QRegExp whiteSpaceRegex(const QString& text);
    void assertAlike(const QString& expected, const QString& actual);
    QString construct(const QString included, const QString includer);
};

}}

#endif // QTEST_INCLUDEWRITERTEST_H_INCLUDED
