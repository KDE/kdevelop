#include <kdebug.h>
#include <kinstance.h>


#include "partselector_factory.h"
#include "partselector_part.h"


extern "C" 
{

  void *init_libkdevpartselector()
    {
      return new PartSelectorFactory;
    }
    
};


PartSelectorFactory::PartSelectorFactory(QObject *parent, const char *name)
  : KDevFactory(parent, name)
{
}


PartSelectorFactory::~PartSelectorFactory()
{
  delete s_instance;
  s_instance = 0;
}


KDevPart *PartSelectorFactory::createPartObject(KDevApi *api, QObject *parent, 
											   const QStringList &/*args*/)
{
  return new PartSelectorPart(api, parent, "partselector");
}


KInstance *PartSelectorFactory::s_instance = 0;

KInstance *PartSelectorFactory::instance()
{
  if (!s_instance)
    s_instance = new KInstance("kdevpartselector");

  return s_instance;
}


#include "partselector_factory.moc"
