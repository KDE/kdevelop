#include <kdebug.h>
#include <kinstance.h>
#include "main.h"
#include "grepview.h"


extern "C" {

    void *init_libkdevgrepview()
    {
        return new GrepFactory;
    }
    
};


GrepFactory::GrepFactory(QObject *parent, const char *name)
    : KLibFactory(parent, name)
{
    instance();
}


GrepFactory::~GrepFactory()
{
    delete s_instance;
    s_instance = 0;
}


QObject *GrepFactory::create(QObject *parent, const char *name,
                             const char *classname, const QStringList &args)
{
    kdDebug(9001) << "Building GrepView" << endl;
    
    QObject *obj = new GrepView(parent, name);
    emit objectCreated(obj);
    return obj;
}


KInstance *GrepFactory::s_instance = 0;
KInstance *GrepFactory::instance()
{
    if (!s_instance)
        s_instance = new KInstance("kdevgrepview");

    return s_instance;
}
