#ifndef _CVSFACTORY_H_
#define _CVSFACTORY_H_

#include <klibloader.h>


class CvsFactory : public KLibFactory
{
    Q_OBJECT

public:
    CvsFactory( QObject *parent=0, const char *name=0 );
    ~CvsFactory();

    virtual QObject* create( QObject *parent, const char *name,
                             const char *classname, const QStringList &args);
    static KInstance *instance();

private:
    static KInstance *s_instance;
};

#endif
