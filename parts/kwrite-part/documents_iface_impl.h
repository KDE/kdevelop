#ifndef __DOCUMENTS_IFACE_IMPL_H__
#define __DOCUMENTS_IFACE_IMPL_H__


#include "keditor/editor.h"
#include "keditor/documents_iface.h"


class DocumentsIfaceImpl : public KEditor::DocumentsEditorIface
{
  Q_OBJECT

public:

  DocumentsIfaceImpl(KEditor::Editor *editor);

  QList<KEditor::Document> documents() const;


};


#endif
