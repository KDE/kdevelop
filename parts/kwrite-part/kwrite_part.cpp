#include <kinstance.h>
#include <kparts/partmanager.h>
#include <kdebug.h>


#include "kwrite/kwview.h"


#include "kwrite_part.h"
#include "kwrite_factory.h"


#include "documents_iface_impl.h"


KWritePart::KWritePart(QObject *parent, const char *name )
  : KEditor::Editor(parent, name), _currentDocument(0)
{
  setInstance(KWritePartFactory::instance());

  // create the interfaces
  (void) new DocumentsIfaceImpl(this);

  setXMLFile("kwriteeditor_part.rc");

  _documents.setAutoDelete(true);
}


KWritePart::~KWritePart()
{
}


KEditor::Document *KWritePart::getDocument(const QString &filename)
{
  DocumentImpl *impl = 0;

  // look for an existing document with that name
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

  // if there was none, create a new one
  bool created = false;
  if (!impl)
  {
    impl = new DocumentImpl(this);

    if (!filename.isEmpty())
	impl->load(filename);
	
    connect(impl->manager(), SIGNAL(activePartChanged(KParts::Part*)), this, SLOT(activePartChanged(KParts::Part*)));

    _documents.append(impl);
    _currentDocument = impl;

    created = true;
  }

  if (created)
	emit Editor::documentAdded();
  
  emit Editor::activatePart(impl);
   
  if (impl->widget())
    emit Editor::activateView(impl->widget());
	
  return impl;
}


void KWritePart::activePartChanged(KParts::Part *part)
{
kdDebug() << "NEW CURRENT DOCUMENT: " << part << endl;

  if (part && part->inherits("KEditor::Document"))
	_currentDocument = (KEditor::Document*)part;
  else
	_currentDocument = 0;

  emit Editor::documentActivated(_currentDocument);
}


KEditor::Document *KWritePart::currentDocument()
{
  return _currentDocument;
}


#include "kwrite_part.moc"
