#include <kdebug.h>
#include <kinstance.h>
#include "main.h"
#include "mdimainfrmcomponent.h"


extern "C" {

    void *init_libkdevqextmdimainfrmview()
    {
        return new MdiMainFrmFactory;
    }
    
};


MdiMainFrmFactory::MdiMainFrmFactory(QObject *parent, const char *name)
    : KLibFactory(parent, name)
{
    instance();
}


MdiMainFrmFactory::~MdiMainFrmFactory()
{
    delete s_instance;
    s_instance = 0;
}


QObject *MdiMainFrmFactory::create(QObject *parent, const char *name,
                                const char *classname, const QStringList &args)
{
    // Depending on classname, this should method should also
    // be able to create the config dialog

    kdDebug(9005) << "Building MdiMainFrameComponent" << endl;

    QObject *obj = new MdiMainFrmComponent(parent, name);
    emit objectCreated(obj);
    return obj;
}


KInstance *MdiMainFrmFactory::s_instance = 0;
KInstance *MdiMainFrmFactory::instance()
{
    if (!s_instance)
        s_instance = new KInstance("kdevqextmdimainfrmview");

    return s_instance;
}
