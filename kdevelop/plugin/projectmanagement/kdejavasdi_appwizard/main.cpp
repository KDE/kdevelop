#include "main.h"
#include "kdejavasdiappwizardplugin.h"


extern "C" {

  void *init_libkdevkdejavasdiappwizard()
  {
    return new KDEJavaSDIAppWizardFactory;
    }

};

KDEJavaSDIAppWizardFactory::KDEJavaSDIAppWizardFactory(QObject *parent, const char *name)
    : KLibFactory(parent, name)
{
}


KDEJavaSDIAppWizardFactory::~KDEJavaSDIAppWizardFactory()
{}


QObject *KDEJavaSDIAppWizardFactory::create(QObject *parent, const char *name,
                              const char *classname, const QStringList &args)
{


    QObject *obj = new KDEJavaSDIAppWizardPlugin(parent, name );
    emit objectCreated(obj);
    return obj;
}
#include "main.moc"
