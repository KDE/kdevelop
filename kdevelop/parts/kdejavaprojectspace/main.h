#ifndef _KDEPROJECTSPACEFACTORY_H_
#define _KDEPROJECTSPACEFACTORY_H_

#include <klibloader.h>


class KDEJavaProjectSpaceFactory : public KLibFactory
{
    Q_OBJECT

public:
    KDEJavaProjectSpaceFactory( QObject *parent=0, const char *name=0 );
    ~KDEJavaProjectSpaceFactory();

    virtual QObject* create( QObject *parent, const char *name,
                             const char *classname, const QStringList &args);
    static KInstance *instance();

private:
    static KInstance *s_instance;
};

#endif
