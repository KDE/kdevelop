#include "edit_iface.h"


KEditor::EditDocumentIface::EditDocumentIface( KEditor::Document *parent, KEditor::Editor *editor)
  : KEditor::DocumentInterface(parent, editor)
{
}


KEditor::EditDocumentIface *KEditor::EditDocumentIface::interface(KEditor::Document *doc)
{
  return static_cast<KEditor::EditDocumentIface*>(doc->queryInterface("KEditor::DocumentIface"));
}


#include "edit_iface.moc"
