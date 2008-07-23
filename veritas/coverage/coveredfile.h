
#ifndef VERITAS_COVEREDFILE_H
#define VERITAS_COVEREDFILE_H

#include <QList>
#include <KUrl>
#include <KDebug>

namespace Veritas
{

class CoveredFile
{
public:
    CoveredFile() : m_nrofLines(0), m_nrofInstrumentedLines(0) {}
    QList<int> m_lines;
    QList<int> m_nrofCalls;
    int m_nrofLines;
    int m_nrofInstrumentedLines;
    KUrl m_sourceLoc;
    void print() {
        kDebug() << m_sourceLoc << m_nrofLines << m_nrofInstrumentedLines;
        kDebug() << m_lines << m_nrofCalls;
    }
};

}

#endif
