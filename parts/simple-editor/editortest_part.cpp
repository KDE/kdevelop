#include <kinstance.h>


#include "editortest_part.h"
#include "editortest_factory.h"


EditorTestPart::EditorTestPart(QObject *parent, const char *name)
  : KEditor::Editor(parent, name)
{
  setInstance( EditorTestPartFactory::instance() );

  setXMLFile("editortest_part.rc");
}


EditorTestPart::~EditorTestPart()
{
}


KEditor::Document *EditorTestPart::document(const KURL &url)
{
  QListIterator<DocumentImpl> it(_documents);
  for ( ; it.current(); ++it)
    if (it.current()->url() == url)
      return it.current();
     
  return 0;
}


KEditor::Document *EditorTestPart::createDocument(QWidget *parentWidget, const KURL &url)
{
  DocumentImpl *impl = new DocumentImpl(this, parentWidget);
  if (!url.isEmpty())
    impl->openURL(url);
             
  _documents.append(impl);
  connect(impl, SIGNAL(destroyed()), this, SLOT(documentDestroyed()));
                 
  return impl;
}


void EditorTestPart::documentDestroyed()
{
  _documents.remove(static_cast<const DocumentImpl*>(sender()));
}
 

#include "editortest_part.moc"
