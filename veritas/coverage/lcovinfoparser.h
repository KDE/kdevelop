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

class VERITAS_COVERAGE_EXPORT LcovInfoParser : public QObject
{
Q_OBJECT
public:
    LcovInfoParser(QObject* parent=0);
    ~LcovInfoParser();
    void setSource(const KUrl&);
    void setSource(QIODevice*);
    void parseLine(const QString& line);
    QList<CoveredFile*> go();

Q_SIGNALS:
    void parsedCoverageData(CoveredFile*);
    void finished();

public Q_SLOTS:
    void parseLines(const QStringList& lines);

private:
    QIODevice* m_sourceDev;
    CoveredFile* m_current;
    QList<CoveredFile*> m_files;

    // scratch variables
    char tmp_firstChar;
    char tmp_secondChar;
    QStringList tmp_s;
    QStringList tmp_l; // temp variables
    QString tmp_f;
};

}

#endif // VERITAS_LCOVINFOPARSER_H
