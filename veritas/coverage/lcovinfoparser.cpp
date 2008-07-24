/* KDevelop xUnit plugin
 *    Copyright 2008 Manuel Breugelmans <mbr.nxi@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */


#include "lcovinfoparser.h"
#include <QFile>
#include <QTextStream>
#include <QStringList>
#include "coveredfile.h"

using Veritas::LcovInfoParser;
using Veritas::CoveredFile;

void LcovInfoParser::setSource(const KUrl& source)
{
    m_source = source;
}

QList<CoveredFile*> LcovInfoParser::go()
{
// #   SF:<absolute path to the source file>
// #   FN:<line number of function start>,<function name> for each function
// #   DA:<line number>,<execution count> for each instrumented line
// #   LH:<number of lines with an execution count> greater than 0
// #   LF:<number of instrumented lines>

// SF:/usr/include/QtCore/qstringlist.h
// FN:69,_ZN11QStringListD1Ev
// FN:73,_ZN11QStringListC1ERKS_
// FN:69,_ZN11QStringListaSERKS_
// FN:85,_ZNK11QStringListplERKS_
// FN:72,_ZN11QStringListC1ERK7QString
// DA:69,36
// DA:71,113
// DA:72,88
// DA:73,90
// DA:74,0
// DA:85,0
// DA:86,0
// DA:88,24
// DA:90,0
// LF:9
// LH:5
// end_of_record


    QFile f(m_source.pathOrUrl());
    f.open(QIODevice::ReadOnly);
    QTextStream str(&f);

    QList<CoveredFile*> files;
    CoveredFile* file;
    while (!str.atEnd()) {
        QString line = str.readLine();
        if (line.startsWith("SF")) {
            file = new CoveredFile;
            file->m_sourceLoc = KUrl(line.split(":").value(1));
        } else if (line.startsWith("DA")) {
            kDebug() << line;
            QStringList l = line.split(":");
            QStringList s = l.value(1).split(",");
            file->m_lines.push_back(s.value(0).toInt());
            file->m_nrofCalls.push_back(s.value(1).toInt());
        } else if (line.startsWith("LF")) {
            file->m_nrofLines = line.split(":").value(1).toInt();
        } else if (line.startsWith("LH")) {
            file->m_nrofInstrumentedLines = line.split(":").value(1).toInt();
        } else if (line.startsWith("end_of_record")) {
            files.push_back(file);
        }
    }
    return files;
}


