#include <kdebug.h>


#include "keditor/editor.h"


#include "editortest_part.h"
#include "documents_iface_impl.h"


using namespace KEditor;


DocumentsIfaceImpl::DocumentsIfaceImpl(Editor *editor)
  : DocumentsEditorIface(editor)
{
}


QList<Document> DocumentsIfaceImpl::documents() const
{
  QList<Document> retval;
  
  QListIterator<DocumentImpl> it(((EditorTestPart*)editor())->_documents);
  for ( ; it.current(); ++it)
    retval.append(it.current());
  
  return retval; 
}


#include "documents_iface_impl.moc"
