#ifndef _KDEBINARYPROJECTFACTORY_H_
#define _KDEBINARYPROJECTFACTORY_H_

#include <klibloader.h>


class KDEBinaryProjectFactory : public KLibFactory
{
    Q_OBJECT

public:
    KDEBinaryProjectFactory( QObject *parent=0, const char *name=0 );
    ~KDEBinaryProjectFactory();

    virtual QObject* create( QObject *parent, const char *name,
                             const char *classname, const QStringList &args);
};

#endif
