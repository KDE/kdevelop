#include "editor.h"


#include "status_iface.h"
#include "KEditorStatusIface.h"


KEditor::StatusDocumentIface::StatusDocumentIface(KEditor::Document *parent, KEditor::Editor *editor)
  : KEditor::DocumentInterface(parent, editor)
{
  m_dcopIface = new KEditor::StatusDocumentDCOPIface(this);
}


DCOPRef KEditor::StatusDocumentIface::dcopInterface() const
{
  return DCOPRef(m_dcopIface);
}


KEditor::StatusDocumentIface *KEditor::StatusDocumentIface::interface(KEditor::Document *doc)
{
  return static_cast<KEditor::StatusDocumentIface*>(doc->queryInterface("KEditor::StatusDocumentIface"));
}
//#include "status_iface.moc.cpp"
