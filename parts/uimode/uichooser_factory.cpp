#include <kdebug.h>
#include <kinstance.h>


#include "uichooser_factory.h"
#include "uichooser_part.h"


extern "C" 
{

  void *init_libkdevuichooser()
    {
      return new UIChooserFactory;
    }
    
};


UIChooserFactory::UIChooserFactory(QObject *parent, const char *name)
  : KDevFactory(parent, name)
{
}


UIChooserFactory::~UIChooserFactory()
{
  delete s_instance;
  s_instance = 0;
}


KDevPart *UIChooserFactory::createPartObject(KDevApi *api, QObject *parent, 
											   const QStringList &/*args*/)
{
  return new UIChooserPart(api, parent, "uichooser");
}


KInstance *UIChooserFactory::s_instance = 0;

KInstance *UIChooserFactory::instance()
{
  if (!s_instance)
    s_instance = new KInstance("kdevuichooser");

  return s_instance;
}


#include "uichooser_factory.moc"
