#include "main.h"
#include "kdeprojectspace.h"
#include <kinstance.h>


extern "C" {

  void *init_libkdevkdeprojectspace()
  {
    return new KDEProjectSpaceFactory;
    }
    
};


KDEProjectSpaceFactory::KDEProjectSpaceFactory(QObject *parent, const char *name)
    : KLibFactory(parent, name){
  instance();
}


KDEProjectSpaceFactory::~KDEProjectSpaceFactory(){
   delete s_instance;
   s_instance = 0;
}


QObject *KDEProjectSpaceFactory::create(QObject *parent, const char *name,
                                        const char */*classname*/, const QStringList &/*args*/)
{
    QObject *obj = new KDEProjectSpace (parent, name );
    emit objectCreated(obj);
    return obj;
}

KInstance *KDEProjectSpaceFactory::s_instance = 0;
KInstance *KDEProjectSpaceFactory::instance()
{
    if (!s_instance)
        s_instance = new KInstance("kdevkdeprojectspace");

    return s_instance;
}
#include "main.moc"
