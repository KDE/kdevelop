#include <kdebug.h>
#include <kinstance.h>


#include "konsoleviewfactory.h"
#include "konsoleviewpart.h"


extern "C" 
{

  void *init_libkdevkonsoleview()
    {
      return new KonsoleViewFactory;
    }
    
};


KonsoleViewFactory::KonsoleViewFactory(QObject *parent, const char *name)
  : KDevFactory(parent, name)
{
}


KonsoleViewFactory::~KonsoleViewFactory()
{
  delete s_instance;
  s_instance = 0;
}


KDevPart *KonsoleViewFactory::createPartObject(KDevApi *api, QObject *parent, 
											   const QStringList &/*args*/)
{
  kdDebug(9001) << "Building KonsoleView" << endl;
  return new KonsoleViewPart(api, parent, "konsole view part");
}


KInstance *KonsoleViewFactory::s_instance = 0;

KInstance *KonsoleViewFactory::instance()
{
  if (!s_instance)
    s_instance = new KInstance("kdevkonsoleview");

  return s_instance;
}


#include "konsoleviewfactory.moc"
