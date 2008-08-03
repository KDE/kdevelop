#ifndef VERITAS_COVEREDFILETEST_H_INCLUDED
#define VERITAS_COVEREDFILETEST_H_INCLUDED

#include <QtCore/QObject>

namespace Veritas
{

class CoveredFile;
class CoveredFileTest : public QObject
{
Q_OBJECT
private slots:
    void init();
    void cleanup();

    void construct();
    void sunny();
    void zeroSloc();
    void callCount();
    void linesSet();
    void isReachable();

private:
    CoveredFile* m_file;
};

}

#endif // VERITAS_COVEREDFILETEST_H_INCLUDED
