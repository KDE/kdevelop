#include <kdebug.h>
#include <kinstance.h>
#include "main.h"
#include "cvsinterface.h"


extern "C" {

    void *init_libkdevcvsinterface()
    {
        return new CvsFactory;
    }
    
};


CvsFactory::CvsFactory(QObject *parent, const char *name)
    : KLibFactory(parent, name)
{
    instance();
}


CvsFactory::~CvsFactory()
{
    delete s_instance;
    s_instance = 0;
}


QObject *CvsFactory::create(QObject *parent, const char *name,
                            const char *classname, const QStringList &args)
{
    kdDebug(9006) << "Building CvsInterface" << endl;
    
    QObject *obj = new CvsInterface(parent, name);
    emit objectCreated(obj);
    return obj;
}


KInstance *CvsFactory::s_instance = 0;
KInstance *CvsFactory::instance()
{
    if (!s_instance)
        s_instance = new KInstance("kdevcvsinterface");

    return s_instance;
}
