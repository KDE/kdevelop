#include <kinstance.h>
#include <kparts/partmanager.h>
#include <kdebug.h>


#include "kwrite/kwview.h"


#include "kwrite_part.h"
#include "kwrite_factory.h"


KWritePart::KWritePart(QObject *parent, const char *name )
  : KTextEditor::Editor(parent, name)
{
  setInstance(KWritePartFactory::instance());

  setXMLFile("kwriteeditor_part.rc");
}


KWritePart::~KWritePart()
{
}


KTextEditor::Document *KWritePart::document(const KURL &url)
{
  QListIterator<DocumentImpl> it(_documents);
  for ( ; it.current(); ++it)
    if (it.current()->url() == url)
      return it.current();

  return 0;
}


KTextEditor::Document *KWritePart::createDocument(QWidget *parentWidget, const KURL &url)
{
  DocumentImpl *impl = new DocumentImpl(this, parentWidget);
  if (!url.isEmpty())
    impl->openURL(url);

  _documents.append(impl);
  connect(impl, SIGNAL(destroyed()), this, SLOT(documentDestroyed()));
  
  return impl;
}


void KWritePart::documentDestroyed()
{
  _documents.remove(static_cast<const DocumentImpl*>(sender()));
}


#include "kwrite_part.moc"
