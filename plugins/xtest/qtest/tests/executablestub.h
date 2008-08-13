#ifndef QTEST_EXECUTABLE_STUB_H
#define QTEST_EXECUTABLE_STUB_H

#include "../executable.h"

namespace QTest { namespace Test {

class ExecutableStub : public QTest::Executable
{
public:
    ExecutableStub() : m_fetchFunctions(0) {}
    virtual ~ExecutableStub() {}
    virtual QStringList fetchFunctions() {
        return m_fetchFunctions;
    }
    QStringList m_fetchFunctions;
    virtual QString name() const {
        return m_name;
    }
    QString m_name;
    virtual KUrl location() const {
        return KUrl();
    }
};

}}

#endif // QTEST_EXECUTABLE_STUB_H
