#include "editor.h"


#include "debug_iface.h"
#include "KEditorDebugIface.h"


KEditor::DebugDocumentIface::DebugDocumentIface(KEditor::Document *parent, KEditor::Editor *editor)
  : KEditor::DocumentInterface(parent, editor)
{
  m_dcopIface = new DebugDocumentDCOPIface(this);
}


DCOPRef KEditor::DebugDocumentIface::dcopInterface() const
{
  return DCOPRef(m_dcopIface);
}


KEditor::DebugDocumentIface *KEditor::DebugDocumentIface::interface(KEditor::Document *doc)
{
  return static_cast<KEditor::DebugDocumentIface*>(doc->queryInterface("KEditor::DebugDocumentIface"));
}
//#include "debug_iface.moc.cpp"
