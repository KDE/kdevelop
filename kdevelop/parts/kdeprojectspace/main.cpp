#include "main.h"
#include "kdeprojectspaceplugin.h"


extern "C" {

  void *init_libkdevkdeprojectspace()
  {
    return new KDEProjectSpaceFactory;
    }
    
};


KDEProjectSpaceFactory::KDEProjectSpaceFactory(QObject *parent, const char *name)
    : KLibFactory(parent, name)
{
}


KDEProjectSpaceFactory::~KDEProjectSpaceFactory()
{}


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
