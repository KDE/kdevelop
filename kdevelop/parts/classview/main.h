#ifndef _CLASSFACTORY_H_
#define _CLASSFACTORY_H_

#include <klibloader.h>


class ClassFactory : public KLibFactory
{
    Q_OBJECT

public:
    ClassFactory( QObject *parent=0, const char *name=0 );
    ~ClassFactory();

    virtual QObject* create(QObject *parent, const char *name,
                            const char *classname, const QStringList &args);
    static KInstance *instance();

private:
    static KInstance *s_instance;
};

#endif
