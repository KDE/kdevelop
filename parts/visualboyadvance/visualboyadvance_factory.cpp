#include <kdebug.h>
#include <kinstance.h>


#include "visualboyadvance_factory.h"
#include "visualboyadvance_part.h"

using namespace VisualBoyAdvance;

extern "C" 
{
  
  void *init_libkdevvisualboyadvance()
  {
    return new VisualBoyAdvanceFactory;
  }
  
};
  

VisualBoyAdvanceFactory::VisualBoyAdvanceFactory(QObject *parent, const char *name)
  : KDevFactory(parent, name)
{
}
  

VisualBoyAdvanceFactory::~VisualBoyAdvanceFactory()
{
    delete s_instance;
    s_instance = 0;
}


KDevPart *VisualBoyAdvanceFactory::createPartObject(KDevApi *api, QObject *parent, 
						    const QStringList &/*args*/)
{
  return new VisualBoyAdvancePart(api, parent, "visualboyadvance");
}
  

KInstance *VisualBoyAdvanceFactory::s_instance = 0;

KInstance *VisualBoyAdvanceFactory::instance()
{
  if (!s_instance)
    s_instance = new KInstance("kdevvisualboyadvance");
  
  return s_instance;
}


#include "visualboyadvance_factory.moc"
