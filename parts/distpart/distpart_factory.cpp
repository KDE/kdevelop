#include <kdebug.h>
#include <kinstance.h>


#include "distpart_factory.h"
#include "distpart_part.h"


extern "C" 
{

  void *init_libkdevdistpart()
    {
      return new distpartFactory;
    }

};


distpartFactory::distpartFactory(QObject *parent, const char *name)
  : KDevFactory(parent, name)
{
}


distpartFactory::~distpartFactory()
{
  delete s_instance;
  s_instance = 0;
}


KDevPart *distpartFactory::createPartObject(KDevApi *api, QObject *parent,
											   const QStringList &/*args*/)
{
  return new distpartPart(api, parent, "distpart");
}


KInstance *distpartFactory::s_instance = 0;

KInstance *distpartFactory::instance()
{
  if (!s_instance)
    s_instance = new KInstance("kdevdistpart");

  return s_instance;
}


#include "distpart_factory.moc"
