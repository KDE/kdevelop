#include <kdebug.h>


#include "keditor/editor.h"


#include "kwrite_part.h"
#include "documents_iface_impl.h"


using namespace KEditor;


DocumentsIfaceImpl::DocumentsIfaceImpl(Editor *editor)
  : DocumentsEditorIface(editor)
{
}


QList<Document> DocumentsIfaceImpl::documents()
{
  QList<Document> retval;
  
  QListIterator<DocumentImpl> it(((KWritePart*)editor())->_documents);
  for ( ; it.current(); ++it)
	{
      kdDebug() << "ADDED to retval: " << it.current()->shortName() << endl;
	  retval.append(it.current());
    }
  
  return retval; 
}


#include "documents_iface_impl.moc"
