#include "main.h"
#include "kdejavaprojectspace.h"
#include <kinstance.h>


extern "C" {

  void *init_libkdevkdejavaprojectspace()
  {
    return new KDEJavaProjectSpaceFactory;
    }
    
};


KDEJavaProjectSpaceFactory::KDEJavaProjectSpaceFactory(QObject *parent, const char *name)
    : KLibFactory(parent, name){
  instance();
}


KDEJavaProjectSpaceFactory::~KDEJavaProjectSpaceFactory(){
   delete s_instance;
   s_instance = 0;
}


QObject *KDEJavaProjectSpaceFactory::create(QObject *parent, const char *name,
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

    QObject *obj = new KDEJavaProjectSpace (parent, name );
    emit objectCreated(obj);
    return obj;
}

KInstance *KDEJavaProjectSpaceFactory::s_instance = 0;
KInstance *KDEJavaProjectSpaceFactory::instance()
{
    if (!s_instance)
        s_instance = new KInstance("kdevkdejavaprojectspace");

    return s_instance;
}
#include "main.moc"
