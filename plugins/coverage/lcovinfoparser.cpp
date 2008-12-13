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
#include <KDebug>
#include "coveredfile.h"


using Veritas::LcovInfoParser;
using Veritas::CoveredFile;

LcovInfoParser::LcovInfoParser(QObject* parent) : QObject(parent), m_sourceDev(0), m_current(0)
{}

LcovInfoParser::~LcovInfoParser()
{
    foreach(CoveredFile* f, m_files) {
        f->deleteLater();
    }
    if (m_sourceDev) delete m_sourceDev;
}

void LcovInfoParser::fto_setSource(QIODevice* device)
{
    m_sourceDev = device;
}

namespace
{

inline bool firstTwoCharsEqual(const char* one, char* two)
{
    return !qstrncmp(one, two, 2);
}

}

void LcovInfoParser::parseLines(const QStringList& lines)
{
    foreach(const QString& l, lines) parseLine(l);
}

void LcovInfoParser::parseLine(const QString& line)
{
    if (line.count() < 3) return;
    tmp_firstChar = line[0].toAscii();
    tmp_secondChar = line[1].toAscii();
    switch(tmp_firstChar) {
    case 'D': {
        if (tmp_secondChar != 'A') break;
        // DA:<line number>,<execution count> for each instrumented line
        Q_ASSERT(m_current);
        //l = line.split(":");
        tmp_f = line.mid(3, -1);
        tmp_s = tmp_f.split(",");
        int lineNumber = tmp_s.value(0).toInt();
        int callCount = tmp_s.value(1).toInt();
        m_current->setCallCount(lineNumber, callCount);
        break;
    } case 'F': { break;
    } case 'S': {
        if (tmp_secondChar != 'F') break;
        // SF:<absolute path to the source file>
        Q_ASSERT(m_current == 0);
        m_current = new CoveredFile;
        m_current->setUrl(KUrl(line.split(":").value(1)));
        break;
    } case 'L': {
        /*if (tmp_secondChar == 'F') {
            // #   LF:<number of instrumented lines>
            Q_ASSERT(m_current);
            m_current->setSloc(line.split(":").value(1).toInt());
        } else if (tmp_secondChar == 'H') {
            // #   LH:<number of lines with an execution count> greater than 0
            Q_ASSERT(m_current);
            m_current->setInstrumented(line.split(":").value(1).toInt());
        }*/
        break;
    } case 'e': {
        if (line.startsWith("end_of_record")) {
            Q_ASSERT(m_current);
            m_files << m_current;
            emit parsedCoverageData(m_files.last());
            m_current = 0;
        }
        break;
    } default: {}}
}

QList<CoveredFile*> LcovInfoParser::fto_go()
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

    Q_ASSERT(m_sourceDev);
    if (!m_sourceDev->isOpen()) {
        m_sourceDev->open(QIODevice::ReadOnly);
        if (!m_sourceDev->isOpen()) {
            kError() << "Failed to open lcov info file for reading.";
            return QList<CoveredFile*>();
        }
    }
    this->disconnect(this);
    connect(this, SIGNAL(parsedCoverageDataForFile(CoveredFile*)),
            this, SLOT(appendCoverageData(CoveredFile*)));

    QTextStream str(m_sourceDev);

    m_files.clear();
    m_current = 0;

    QString line;
    while (!str.atEnd()) {
        line = str.readLine();
        parseLine(line);
    }

    return m_files;
}

#include "lcovinfoparser.moc"
