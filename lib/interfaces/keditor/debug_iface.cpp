#include "editor.h"


#include "debug_iface.h"


KEditor::DebugDocumentIface::DebugDocumentIface(KEditor::Document *parent, KEditor::Editor *editor)
  : KEditor::DocumentInterface(parent, editor)
{
}


KEditor::DebugDocumentIface *KEditor::DebugDocumentIface::interface(KEditor::Document *doc)
{
  return static_cast<KEditor::DebugDocumentIface*>(doc->queryInterface("KEditor::DebugDocumentIface"));
}
