#include <kdebug.h>
#include <kinstance.h>


#include "editorchooser_factory.h"
#include "editorchooser_part.h"


extern "C" 
{

  void *init_libkdeveditorchooser()
    {
      return new EditorChooserFactory;
    }
    
};


EditorChooserFactory::EditorChooserFactory(QObject *parent, const char *name)
  : KDevFactory(parent, name)
{
}


EditorChooserFactory::~EditorChooserFactory()
{
  delete s_instance;
  s_instance = 0;
}


KDevPart *EditorChooserFactory::createPartObject(KDevApi *api, QObject *parent, 
											   const QStringList &/*args*/)
{
  return new EditorChooserPart(api, parent, "editorchooser");
}


KInstance *EditorChooserFactory::s_instance = 0;

KInstance *EditorChooserFactory::instance()
{
  if (!s_instance)
    s_instance = new KInstance("kdeveditorchooser");

  return s_instance;
}


#include "editorchooser_factory.moc"
