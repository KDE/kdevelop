#ifndef _PRINTFACTORY_H_
#define _PRINTFACTORY_H_

#include <klibloader.h>


class PrintFactory : public KLibFactory
{
    Q_OBJECT

public:
    PrintFactory( QObject *parent=0, const char *name=0 );
    ~PrintFactory();

    virtual QObject* create( QObject *parent, const char *name,
                             const char *classname, const QStringList &args);
};

#endif
