#include "main.h"
#include "javaprojectspace.h"


extern "C" {
  void *init_libjava_projectspace(){
    return new JavaProjectSpaceFactory;
  }
  
};


JavaProjectSpaceFactory::JavaProjectSpaceFactory(QObject *parent, const char *name)
  : KLibFactory(parent, name){
}

JavaProjectSpaceFactory::~JavaProjectSpaceFactory(){
}

QObject *JavaProjectSpaceFactory::create(QObject *parent, const char *name,
                              const char *classname, const QStringList &args){   
  QObject *obj = new JavaProjectSpace (parent, name );
    emit objectCreated(obj);
    return obj;
}
