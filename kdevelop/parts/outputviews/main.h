#ifndef _OUTPUTFACTORY_H_
#define _OUTPUTFACTORY_H_

#include <klibloader.h>


class OutputFactory : public KLibFactory
{
    Q_OBJECT

public:
    OutputFactory( QObject *parent=0, const char *name=0 );
    ~OutputFactory();

    virtual QObject* create(QObject *parent, const char *name,
                            const char *classname, const QStringList &args);
    static KInstance *instance();

private:
    static KInstance *s_instance;
};

#endif
