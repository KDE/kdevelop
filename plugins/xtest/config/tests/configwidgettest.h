#ifndef QTEST_CONFIGWIDGETTEST_H_INCLUDED
#define QTEST_CONFIGWIDGETTEST_H_INCLUDED

#include <QtCore/QObject>

namespace Veritas
{

class ConfigWidgetTest : public QObject
{
Q_OBJECT
private slots:
    void init();
    void cleanup();

    void construct();
    void expandDetails();
};

}

#endif // QTEST_CONFIGWIDGETTEST_H_INCLUDED
