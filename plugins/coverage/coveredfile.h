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

#ifndef VERITAS_COVEREDFILE_H
#define VERITAS_COVEREDFILE_H

#include "coverageexport.h"

#include <QList>
#include <KUrl>
#include <QMap>
#include <KDebug>

namespace Veritas
{

/*! Prime value class, holds coverage data for a file. This
 * includes the covered lines, uncovered ones and callcounts */
class VERITAS_COVERAGE_EXPORT CoveredFile : public QObject
{
Q_OBJECT
public:
    CoveredFile();
    virtual ~CoveredFile();

    KUrl url() const;
    void setUrl(const KUrl& url);
    void setCallCount(int line, int count);
    /*! line coverage percentage */
    double coverage() const;
    /*! number of source lines of code */
    int sloc() const;
    //void setSloc(int nrofLines); // TODO should be removed

    /*! number of visitted lines of code */
    int instrumented() const;
    //void setInstrumented(int nrofLines); // TODO should be removed

    QMap<int, int> callCountMap() const;
    QSet<int> coveredLines() const;
    QSet<int> reachableLines() const;
    inline bool isReachable(int line) const { 
        return m_reachableLines.contains(line);
    }

private:
    QMap<int, int> m_nrofCalls; // { line x nrofcalls }
    int m_nrofLines;
    int m_nrofInstrumentedLines;
    KUrl m_sourceLoc;
    QSet<int> m_coveredLines;
    QSet<int> m_reachableLines;
};

}

#endif
