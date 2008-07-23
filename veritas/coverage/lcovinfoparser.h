#ifndef VERITAS_LCOVINFOPARSER_H
#define VERITAS_LCOVINFOPARSER_H

#include <QList>
#include <KUrl>

namespace Veritas
{

class CoveredFile;

class LcovInfoParser
{
public:
    void setSource(const KUrl&);
    QList<CoveredFile*> go();

private:
    KUrl m_source;
};

}

#endif // VERITAS_LCOVINFOPARSER_H
