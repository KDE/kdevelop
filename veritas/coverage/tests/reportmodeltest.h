#ifndef VERITAS_COVERAGE_REPORTMODELTEST_H
#define VERITAS_COVERAGE_REPORTMODELTEST_H

#include <QtCore/QObject>
#include <QtCore/QVariant>
#include <KUrl>
class QModelIndex;

namespace Veritas
{

class CoveredFile;
class ReportModel;
class ReportModelTest : public QObject
{
Q_OBJECT
private slots:
    void init();
    void cleanup();
    
    void sortRole();
    void addSingleCoverageData();
    void multiCoverageOneFile();
    void differentSlocSetOneFile();
    
private:
    QVariant fetchSortData(int row);
    void assertDirAtEquals(const QModelIndex& i, QString path, int sloc, int instrumented);
    void assertFileAtEquals(const QModelIndex& i, QString name, int sloc, int instrumented, double cov);

private:
    ReportModel* m_model;
    KUrl m_fileUrl;
    QString m_fileDir;
    QString m_fileName;
};

}

#endif // VERITAS_COVERAGE_REPORTMODELTEST_H
