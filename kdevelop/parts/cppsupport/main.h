#ifndef _CPPSUPPORTFACTORY_H_
#define _CPPSUPPORTFACTORY_H_

#include <klibloader.h>


class CppSupportFactory : public KLibFactory
{
    Q_OBJECT

public:
    CppSupportFactory( QObject *parent=0, const char *name=0 );
    ~CppSupportFactory();

    virtual QObject* create(QObject *parent, const char *name,
                            const char *classname, const QStringList &args);
    static KInstance *instance();

private:
    static KInstance *s_instance;
};

#endif
