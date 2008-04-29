#ifndef CACHEREADER_H
#define CACHEREADER_H

class QString;

#include "cmakeexport.h"

struct CacheLine
{
    CacheLine() : endName(-1), dash(-1), colon(-1), equal(-1) {}
    int endName;
    int dash;
    int colon;
    int equal;
    
    bool isCorrect() const { return endName>=0 && equal>=0; }
};

CacheLine KDEVCMAKECOMMON_EXPORT readLine(const QString& line);

// readCache(const KUrl &path);

#endif
