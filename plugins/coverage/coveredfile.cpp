/* KDevelop coverage plugin
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

#include "coveredfile.h"

using Veritas::CoveredFile;

CoveredFile::CoveredFile()
    : m_nrofLines(0), m_nrofCoveredLines(0)
{}

CoveredFile::~CoveredFile()
{
}

KUrl CoveredFile::url() const
{
    return m_sourceLoc;
}

void CoveredFile::setUrl(const KUrl& url)
{
    m_sourceLoc = url;
}

void CoveredFile::setCallCount(int line, int count)
{
    m_nrofCalls[line] = count;
    m_reachableLines << line;
    m_nrofLines = m_reachableLines.count();
    if (count != 0) {
        m_coveredLines << line;
        m_nrofCoveredLines = m_coveredLines.count();
    }
}

QSet<int> CoveredFile::coveredLines() const
{
    return m_coveredLines;
}

QSet<int> CoveredFile::reachableLines() const
{
    return m_reachableLines;
}

double CoveredFile::coverageRatio() const
{
    if (m_nrofLines == 0) return 0;
    return 100*(double)m_nrofCoveredLines/(double)m_nrofLines;
}

int CoveredFile::sloc() const
{
    return m_nrofLines;
}

int CoveredFile::nrofCoveredLines() const
{
    return m_nrofCoveredLines;
}

QMap<int, int> CoveredFile::callCountMap() const
{
    return m_nrofCalls;
}

#include "coveredfile.moc"
