#include <kdebug.h>
#include <kinstance.h>


#include "fileselector_factory.h"
#include "fileselector_part.h"


extern "C"
{

  void *init_libkdevfileselector()
    {
      return new FileSelectorFactory;
    }

};


FileSelectorFactory::FileSelectorFactory(QObject *parent, const char *name)
  : KDevFactory(parent, name)
{
}


FileSelectorFactory::~FileSelectorFactory()
{
  delete s_instance;
  s_instance = 0;
}


KDevPart *FileSelectorFactory::createPartObject(KDevApi *api, QObject *parent,
											   const QStringList &/*args*/)
{
  return new FileSelectorPart(api, parent, "fileselector");
}


KInstance *FileSelectorFactory::s_instance = 0;

KInstance *FileSelectorFactory::instance()
{
  if (!s_instance)
    s_instance = new KInstance("kdevfileselector");

  return s_instance;
}


#include "fileselector_factory.moc"
