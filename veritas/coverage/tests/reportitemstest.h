#ifndef QTEST_REPORTITEMSTEST_H_INCLUDED
#define QTEST_REPORTITEMSTEST_H_INCLUDED

#include <QtCore/QObject>

namespace Veritas
{

class ReportItemsTest : public QObject
{
Q_OBJECT
private slots:
    void init();
    void cleanup();

    void constructDoubleValueItem();
    void constructIntValueItem();

    void constructDirItem();
    void addFileToDirItem();
    void addMultipleFilesToDir();
};

}

#endif // QTEST_REPORTITEMSTEST_H_INCLUDED
