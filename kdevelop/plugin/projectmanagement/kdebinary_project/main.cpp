#include "main.h"
#include "kdebinaryprojectplugin.h"


extern "C" {
  
  void *init_libkdebinary_project(){
    return new KDEBinaryProjectFactory;
  }
  
};

KDEBinaryProjectFactory::KDEBinaryProjectFactory(QObject *parent, const char *name)
    : KLibFactory(parent, name)
{
}

KDEBinaryProjectFactory::~KDEBinaryProjectFactory()
{}


QObject *KDEBinaryProjectFactory::create(QObject *parent, const char *name,
					 const char *classname, const QStringList &args){   
  QObject *obj = new KDEBinaryProjectPlugin(parent, name );
  emit objectCreated(obj);
  return obj;
}
