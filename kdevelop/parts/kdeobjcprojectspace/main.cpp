#include "main.h"
#include "kdeobjcprojectspace.h"
#include <kinstance.h>


extern "C" {

  void *init_libkdevkdeobjcprojectspace()
  {
    return new KDEObjcProjectSpaceFactory;
    }
    
};


KDEObjcProjectSpaceFactory::KDEObjcProjectSpaceFactory(QObject *parent, const char *name)
    : KLibFactory(parent, name){
  instance();
}


KDEObjcProjectSpaceFactory::~KDEObjcProjectSpaceFactory(){
   delete s_instance;
   s_instance = 0;
}


QObject *KDEObjcProjectSpaceFactory::create(QObject *parent, const char *name,
                              const char *classname, const QStringList &args)
{
    QString filename;
    if (!args.isEmpty())
        filename = *args.begin();

    /*if (!parent->isWidgetType()) {
        qDebug("Parent of  dialog is not a widget");
        return 0;
	}*/
    //    QWidget *parentWidget = (QWidget *) parent;
    
    // Depending on classname, this should method should also
    // be able to create the config dialog
    
    QObject *obj = new KDEObjcProjectSpace (parent, name );
    emit objectCreated(obj);
    return obj;
}

KInstance *KDEObjcProjectSpaceFactory::s_instance = 0;
KInstance *KDEObjcProjectSpaceFactory::instance()
{
    if (!s_instance)
        s_instance = new KInstance("kdevkdeobjcprojectspace");

    return s_instance;
}
#include "main.moc"
