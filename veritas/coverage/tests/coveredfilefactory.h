#ifndef VERITAS_COVERAGE_CREATECOVEREDFILE_H
#define VERITAS_COVERAGE_CREATECOVEREDFILE_H

#include "../coveredfile.h"

namespace Veritas
{
namespace Test
{

class CoveredFileFactory
{
public:
    typedef QList<int> Lines;

    /*! Construct a covered file with @param sloc number of reachable lines and
        @param instrumented covered lines */
    static CoveredFile* create(const KUrl& name, int sloc, int instrumented)
    {
        CoveredFile* f = new CoveredFile;
        f->setUrl(name);
        for (int i=0; i<sloc; i++) {
            f->setCallCount(i,0);
        }
        for (int i=0; i<instrumented; i++) {
            f->setCallCount(i,1);
        }
        return f;
    }

    /*! Construct a covered file with the line numbers in @param notCoveredLines 
        as lines of code with call count zero and those in @param coveredLines with a call
        count greater than zero. */
    static CoveredFile* create(const KUrl& name, const Lines& coveredLines, const Lines& notCoveredLines)
    {
        CoveredFile* f = new CoveredFile;
        f->setUrl(name);
        foreach(int line, notCoveredLines) {
            f->setCallCount(line,0);
        }
        foreach(int line, coveredLines) {
            f->setCallCount(line, 1);
        }
        return f;
    }

    /*! Construct a covered file with given @param callCountMap Keys are line numbers, values the 
        associated call count */
    static CoveredFile* create(const KUrl& name, const QMap<int, int>& callCountMap)
    {
        CoveredFile* f = new CoveredFile;
        f->setUrl(name);
        QMapIterator<int,int> it(callCountMap);
        while(it.hasNext()) {
            it.next();
            f->setCallCount(it.key(), it.value());
        }
        return f;
    }
};

}} // namespace Veritas

#endif