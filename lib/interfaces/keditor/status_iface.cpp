#include "editor.h"


#include "status_iface.h"


KEditor::StatusDocumentIface::StatusDocumentIface(KEditor::Document *parent, KEditor::Editor *editor)
  : KEditor::DocumentInterface(parent, editor)
{
}


KEditor::StatusDocumentIface *KEditor::StatusDocumentIface::interface(KEditor::Document *doc)
{
  return static_cast<KEditor::StatusDocumentIface*>(doc->queryInterface("KEditor::StatusDocumentIface"));
}
