#ifndef KDETEST_H
#define KDETEST_h

#include <QtCore/QObject>

class KdeTest : public QObject
{
    Q_OBJECT
    
private slots:
    void initTestCase();
    
    void passingTestCase();
    void failingTestCase();
    void expectedFailTestCase();
    void unexpectedPassTestCase();
    void skippedTestCase();
    
    void cleanupTestCase();
};

#endif // KDETEST_H
