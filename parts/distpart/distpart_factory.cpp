#include <kdebug.h>
#include <kinstance.h>


#include "distpart_factory.h"
#include "distpart_part.h"


extern "C" 
{

  void *init_libkdevdistpart()
    {
      return new DistpartFactory;
    }

};


DistpartFactory::DistpartFactory(QObject *parent, const char *name)
  : KDevFactory(parent, name)
{
}


DistpartFactory::~DistpartFactory()
{
  delete s_instance;
  s_instance = 0;
}


KDevPart *DistpartFactory::createPartObject(KDevApi *api, QObject *parent,
                                            const QStringList &/*args*/)
{
  return new DistpartPart(api, parent, "distpart");
}


KInstance *DistpartFactory::s_instance = 0;

KInstance *DistpartFactory::instance()
{
  if (!s_instance)
    s_instance = new KInstance("kdevdistpart");

  return s_instance;
}


#include "distpart_factory.moc"
