#include "main.h"
#include "kdeobjcsdiappwizardplugin.h"


extern "C" {

  void *init_libkdevkdeobjcsdiappwizard()
  {
    return new KDEObjcSDIAppWizardFactory;
    }

};

KDEObjcSDIAppWizardFactory::KDEObjcSDIAppWizardFactory(QObject *parent, const char *name)
    : KLibFactory(parent, name)
{
}


KDEObjcSDIAppWizardFactory::~KDEObjcSDIAppWizardFactory()
{}


QObject *KDEObjcSDIAppWizardFactory::create(QObject *parent, const char *name,
                              const char *classname, const QStringList &args)
{


    QObject *obj = new KDEObjcSDIAppWizardPlugin(parent, name );
    emit objectCreated(obj);
    return obj;
}
#include "main.moc"
