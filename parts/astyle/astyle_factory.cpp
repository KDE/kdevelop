#include <kdebug.h>
#include <kinstance.h>


#include "astyle_factory.h"
#include "astyle_part.h"


extern "C" 
{

  void *init_libkdevastyle()
    {
      return new AStyleFactory;
    }
    
};


AStyleFactory::AStyleFactory(QObject *parent, const char *name)
  : KDevFactory(parent, name)
{
}


AStyleFactory::~AStyleFactory()
{
  delete s_instance;
  s_instance = 0;
}


KDevPart *AStyleFactory::createPartObject(KDevApi *api, QObject *parent, const QStringList &/*args*/)
{
  return new AStylePart(api, parent, "astyle");
}


KInstance *AStyleFactory::s_instance = 0;

KInstance *AStyleFactory::instance()
{
  if (!s_instance)
    s_instance = new KInstance("kdevastyle");

  return s_instance;
}


#include "astyle_factory.moc"
