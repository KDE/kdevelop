#include <kdebug.h>
#include <kinstance.h>
#include "main.h"
#include "classview.h"


extern "C" {

    void *init_libkdevclassview()
    {
        return new ClassFactory;
    }
    
};


ClassFactory::ClassFactory(QObject *parent, const char *name)
    : KLibFactory(parent, name)
{
    instance();
}


ClassFactory::~ClassFactory()
{
    delete s_instance;
    s_instance = 0;
}


QObject *ClassFactory::create(QObject *parent, const char *name,
                             const char *classname, const QStringList &args)
{
    kdDebug(9001) << "Building ClassView" << endl;
    
    QObject *obj = new ClassView(parent, name);
    emit objectCreated(obj);
    return obj;
}


KInstance *ClassFactory::s_instance = 0;
KInstance *ClassFactory::instance()
{
    if (!s_instance)
        s_instance = new KInstance("kdevclassview");

    return s_instance;
}
