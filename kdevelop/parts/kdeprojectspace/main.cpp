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
