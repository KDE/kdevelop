#ifndef __DOCUMENTS_IFACE_IMPL_H__
#define __DOCUMENTS_IFACE_IMPL_H__


#include <qmultilineedit.h>


#include "keditor/editor.h"
#include "keditor/documents_iface.h"


namespace KEditor {
		

class DocumentsIfaceImpl : public KEditor::DocumentsEditorIface
{
  Q_OBJECT

public:

  DocumentsIfaceImpl(KEditor::Editor *editor);

  QList<Document> documents();


};


}


#endif
