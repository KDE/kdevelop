#ifndef QTEST_TESTS_STANDALONERUNNER_H
#define QTEST_TESTS_STANDALONERUNNER_H

#include <QObject>
#include <QString>

namespace Veritas { class Test; }
namespace QTest { class XmlRegister; }

class Boot : public QObject
{
Q_OBJECT
public:
    void start();
    QString rootDir;
    QString regXML;

public slots:
    void load();
    void showWindow();

private:
    QTest::XmlRegister* m_reg;
};

#endif // QTEST_TEST_STANDALONERUNNER_H
