#ifndef _GREPFACTORY_H_
#define _GREPFACTORY_H_

#include <klibloader.h>


class GrepFactory : public KLibFactory
{
    Q_OBJECT

public:
    GrepFactory( QObject *parent=0, const char *name=0 );
    ~GrepFactory();

    virtual QObject* create(QObject *parent, const char *name,
                            const char *classname, const QStringList &args);
    static KInstance *instance();

private:
    static KInstance *s_instance;
};

#endif
