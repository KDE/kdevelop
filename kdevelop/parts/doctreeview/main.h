#ifndef _DOCTREEFACTORY_H_
#define _DOCTREEFACTORY_H_

#include <klibloader.h>


class DocTreeFactory : public KLibFactory
{
    Q_OBJECT

public:
    DocTreeFactory( QObject *parent=0, const char *name=0 );
    ~DocTreeFactory();

    virtual QObject* create( QObject *parent, const char *name,
                             const char *classname, const QStringList &args);
    static KInstance *instance();

private:
    static KInstance *s_instance;
};

#endif
