#ifndef QTEST_REPORTFILEITEMTEST_H_INCLUDED
#define QTEST_REPORTFILEITEMTEST_H_INCLUDED

#include <QtCore/QObject>
#include <KUrl>
#include <QString>

namespace Veritas
{

class ReportFileItem;
class ReportFileItemTest : public QObject
{
Q_OBJECT
private slots:
    void init();
    void cleanup();

    void construct();
    void addCoverage();
    void addMultipleCoverage();

private:
    void assertInstrumentedItem(int);
    void assertCoverageItem(double);
    void assertSlocItem(int);

private:
    ReportFileItem* m_file;
    KUrl m_url;
    QString m_fileName;
};

}

#endif // QTEST_REPORTFILEITEMTEST_H_INCLUDED
