#include <kdebug.h>


#include "keditor/editor.h"


#include "kwrite_part.h"
#include "documents_iface_impl.h"


DocumentsIfaceImpl::DocumentsIfaceImpl(KEditor::Editor *editor)
  : DocumentsEditorIface(editor)
{
}


QList<KEditor::Document> DocumentsIfaceImpl::documents() const
{
  QList<KEditor::Document> retval;
 /* 
  QListIterator<DocumentImpl> it(((KWritePart*)editor())->_documents);
  for ( ; it.current(); ++it)
	{
      kdDebug() << "ADDED to retval: " << it.current()->shortName() << endl;
	  retval.append(it.current());
    }
  */
  return retval; 
}


#include "documents_iface_impl.moc"
