#ifndef QTEST_LCOVINFOPARSERTEST_H_INCLUDED
#define QTEST_LCOVINFOPARSERTEST_H_INCLUDED

#include <QtCore/QObject>

namespace Veritas
{

class CoveredFile;
class LcovInfoParser;
class LcovInfoParserTest : public QObject
{
Q_OBJECT
private slots:
    void init();
    void cleanup();

    void singleCoveredFile();
    void multipleFiles();

private:
    void assertCoveredFilesEqual(CoveredFile*, CoveredFile*);

private:
    LcovInfoParser* m_parser;
};

}

#endif // QTEST_LCOVINFOPARSERTEST_H_INCLUDED
