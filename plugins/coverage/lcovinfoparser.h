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

#ifndef VERITAS_LCOVINFOPARSER_H
#define VERITAS_LCOVINFOPARSER_H

#include <QIODevice>
#include <QList>
#include <QStringList>
#include <KUrl>
#include "coverageexport.h"

namespace Veritas
{

class CoveredFile;

/*! Parses lcov output into CoveredFile* and emits those as soon
 * as they are read 
 * @note instantiate a fresh parser for every lcov.info file */
class VERITAS_COVERAGE_EXPORT LcovInfoParser : public QObject
{
Q_OBJECT
public:
    LcovInfoParser(QObject* parent=0);
    ~LcovInfoParser();

    /*! Parse a single line of lcov output. The results are
     *  emitted through `parsedCoverageData(CoveredFile*)' */
    void parseLine(const QString& line);
    
    QList<CoveredFile*> fto_coveredFiles();
    
Q_SIGNALS:
    /*! Emitted whenever a full coverage record was parsed.
     *  @p file contains the parsed coverage information */
    void parsedCoverageData(CoveredFile* file);
    
public Q_SLOTS:
    void parseLines(const QStringList& lines);

private:
    CoveredFile* m_current;
    QList<CoveredFile*> m_files;

    // scratch variables
    char tmp_firstChar;
    char tmp_secondChar;
    QStringList tmp_s;
    QStringList tmp_l;
    QString tmp_f;
};

}

#endif // VERITAS_LCOVINFOPARSER_H
