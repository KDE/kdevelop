#ifndef _GCCOPTIONSFACTORY_H_
#define _GCCOPTIONSFACTORY_H_

#include <klibloader.h>


class GccOptionsFactory : public KLibFactory
{
    Q_OBJECT

public:
    GccOptionsFactory( QObject *parent=0, const char *name=0 );
    ~GccOptionsFactory();

    virtual QObject* create( QObject *parent, const char *name,
                             const char *classname, const QStringList &args);
    static KInstance *instance();

private:
    static KInstance *s_instance;
};

#endif
