#ifndef _DBGFACTORY_H_
#define _DBGFACTORY_H_

#include <klibloader.h>


class DbgFactory : public KLibFactory
{
    Q_OBJECT

public:
    DbgFactory( QObject *parent=0, const char *name=0 );
    ~DbgFactory();

    virtual QObject* create(QObject *parent, const char *name,
                            const char *classname, const QStringList &args);
    static KInstance *instance();

private:
    static KInstance *s_instance;
};

#endif
