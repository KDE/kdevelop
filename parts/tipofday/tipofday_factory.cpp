#include <kdebug.h>
#include <kinstance.h>


#include "tipofday_factory.h"
#include "tipofday_part.h"


extern "C" 
{

  void *init_libkdevtipofday()
    {
      return new TipOfDayFactory;
    }
    
};


TipOfDayFactory::TipOfDayFactory(QObject *parent, const char *name)
  : KDevFactory(parent, name)
{
}


TipOfDayFactory::~TipOfDayFactory()
{
  delete s_instance;
  s_instance = 0;
}


KDevPart *TipOfDayFactory::createPartObject(KDevApi *api, QObject *parent, 
											   const QStringList &/*args*/)
{
  return new TipOfDayPart(api, parent, "tipofday");
}


KInstance *TipOfDayFactory::s_instance = 0;

KInstance *TipOfDayFactory::instance()
{
  if (!s_instance)
    s_instance = new KInstance("kdevtipofday");

  return s_instance;
}


#include "tipofday_factory.moc"
