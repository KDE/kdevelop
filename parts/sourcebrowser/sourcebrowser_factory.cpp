#include <kdebug.h>
#include <kinstance.h>


#include "sourcebrowser_factory.h"
#include "sourcebrowser_part.h"


extern "C" 
{

  void *init_libkdevsourcebrowser()
    {
      return new SourceBrowserFactory;
    }
    
};


SourceBrowserFactory::SourceBrowserFactory(QObject *parent, const char *name)
  : KDevFactory(parent, name)
{
}


SourceBrowserFactory::~SourceBrowserFactory()
{
  delete s_instance;
  s_instance = 0;
}


KDevPart *SourceBrowserFactory::createPartObject(KDevApi *api, QObject *parent, 
											   const QStringList &/*args*/)
{
  return new SourceBrowserPart(api, parent, "sourcebrowser");
}


KInstance *SourceBrowserFactory::s_instance = 0;

KInstance *SourceBrowserFactory::instance()
{
  if (!s_instance)
    s_instance = new KInstance("kdevsourcebrowser");

  return s_instance;
}


#include "sourcebrowser_factory.moc"
