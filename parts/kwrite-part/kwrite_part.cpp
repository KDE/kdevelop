#include <kinstance.h>
#include <kparts/partmanager.h>
#include <kdebug.h>


#include "kwrite/kwview.h"


#include "kwrite_part.h"
#include "kwrite_factory.h"


KWritePart::KWritePart(QObject *parent, const char *name )
  : KEditor::Editor(parent, name)
{
  setInstance(KWritePartFactory::instance());

  setXMLFile("kwriteeditor_part.rc");
}


KWritePart::~KWritePart()
{
}


KEditor::Document *KWritePart::document(const KURL &url)
{
  DocumentImpl *impl = 0;
  return impl;

  // impl = KEditor::Editor::getDocument(filename)
 
  // look for an existing document with that name
  /* TODO: Use part manager, put into base class
  if (!filename.isEmpty())
  {
    QListIterator<DocumentImpl> it(_documents);
    for ( ; it.current(); ++it)
    {
      kdDebug() << "Iterator: " << it.current() << endl;
      kdDebug() << "it.current()->fileName: " << it.current()->fileName() << endl;

      if (it.current()->fileName() == filename)
 	{
	  impl = it.current();
	  break;
	}
    }
  }
*/
  // if there was none, create a new one
  if (!impl)
  {
    impl = new DocumentImpl(this);

    if (!url.isEmpty())
	impl->openURL(url);
  }

  return impl;
}


KEditor::Document *KWritePart::createDocument(const KURL &url)
{
  DocumentImpl *impl = new DocumentImpl(this);
  if (!url.isEmpty())
    impl->openURL(url);
  return impl;
}


KEditor::Document *KWritePart::currentDocument()
{
  // TODO: use part manager to look up
  return 0;
}


#include "kwrite_part.moc"
