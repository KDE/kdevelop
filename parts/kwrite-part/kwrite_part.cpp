#include <kinstance.h>
#include <kparts/partmanager.h>


#include "kwrite/kwview.h"


#include "kwrite_part.h"
#include "kwrite_factory.h"


#include "documents_iface_impl.h"


using namespace KEditor;


KWritePart::KWritePart(QObject *parent, const char *name )
  : Editor(parent, name)
{
  setInstance( KWritePartFactory::instance() );

  // create the interfaces
  new DocumentsIfaceImpl(this);

  setXMLFile("kwriteeditor_part.rc");

  _documents.setAutoDelete(true);
}


KWritePart::~KWritePart()
{
}


void KWritePart::currentChanged(QWidget *widget)
{
/*
  QListIterator<DocumentImpl> it(_documents);
  for ( ; it.current(); ++it)
	if (it.current()->widget() == widget)
	  emit documentActivated(it.current());		
*/
}


Document *KWritePart::getDocument(const QString &filename)
{
  DocumentImpl *impl = 0;

  // look for an existing document with that name
  if (!filename.isEmpty())
  {
    QListIterator<DocumentImpl> it(_documents);
    for ( ; it.current(); ++it)
      if (it.current()->fileName() == filename)
 	{
	  impl = it.current();
	  break;
	}
  }

  // if there was none, create a new one
  bool created = false;
  if (!impl)
  {
    impl = new DocumentImpl(this);

    if (!filename.isEmpty())
	impl->load(filename);
	
    connect(impl, SIGNAL(fileNameChanged(QString)), this, SLOT(fileNameChanged(QString)));

    _documents.append(impl);

    created = true;
  }

  if (created)
	emit Editor::documentAdded();
  
  emit Editor::activatePart(impl);
   
  if (impl->widget())
    emit Editor::activateView(impl->widget());
	
  return impl;
}


Document *KWritePart::currentDocument()
{
/*
  QListIterator<DocumentImpl> it(_documents);
  for ( ; it.current(); ++it)
    if (it.current() == it.current()->manager()->activePart())
      return it.current();
*/

  return 0;
}


void KWritePart::fileNameChanged(QString)
{
/*
   QListIterator<DocumentImpl> it(_documents);
   for ( ; it.current(); ++it)
     _stack->changeTab(it.current()->widget(), it.current()->shortName());
*/
}


#include "kwrite_part.moc"
