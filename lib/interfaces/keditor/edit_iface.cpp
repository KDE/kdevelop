#include "edit_iface.h"
#include "KEditorEditIface.h"


KEditor::EditDocumentIface::EditDocumentIface( KEditor::Document *parent, KEditor::Editor *editor)
  : KEditor::DocumentInterface(parent, editor)
{
  m_dcopIface = new KEditor::EditDocumentDCOPIface(this);
}


DCOPRef KEditor::EditDocumentIface::dcopInterface() const
{
  return DCOPRef(m_dcopIface);
}


KEditor::EditDocumentIface *KEditor::EditDocumentIface::interface(KEditor::Document *doc)
{
  return static_cast<KEditor::EditDocumentIface*>(doc->queryInterface("KEditor::EditDocumentIface"));
}


#include "edit_iface.moc"
