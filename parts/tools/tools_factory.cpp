#include <kdebug.h>
#include <kinstance.h>


#include "tools_factory.h"
#include "tools_part.h"


extern "C"
  {

    void *init_libkdevtools()
    {
      return new ToolsFactory;
    }

  };


ToolsFactory::ToolsFactory(QObject *parent, const char *name)
    : KDevFactory(parent, name)
{}



ToolsFactory::~ToolsFactory()
{
  delete s_instance;
  s_instance = 0;
}


KDevPart *ToolsFactory::createPartObject(KDevApi *api, QObject *parent,
    const QStringList &/*args*/)
{
  return new ToolsPart(api, parent, "tools");
}


KInstance *ToolsFactory::s_instance = 0;

KInstance *ToolsFactory::instance()
{
  if (!s_instance)
    s_instance = new KInstance("kdevtools");

  return s_instance;
}


#include "tools_factory.moc"
