#include "main.h"
#include "kdesdiappwizardplugin.h"


extern "C" {

  void *init_libkdesdi_appwizard()
  {
    return new KDESDIAppWizardFactory;
    }
    
};

KDESDIAppWizardFactory::KDESDIAppWizardFactory(QObject *parent, const char *name)
    : KLibFactory(parent, name)
{
}


KDESDIAppWizardFactory::~KDESDIAppWizardFactory()
{}


QObject *KDESDIAppWizardFactory::create(QObject *parent, const char *name,
                              const char *classname, const QStringList &args)
{
   
    
    QObject *obj = new KDESDIAppWizardPlugin(parent, name );
    emit objectCreated(obj);
    return obj;
}
