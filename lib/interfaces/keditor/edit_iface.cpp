#include "edit_iface.h"


KEditor::EditDocumentIface::EditDocumentIface( KEditor::Document *parent, KEditor::Editor *editor)
  : KEditor::DocumentInterface(parent, editor)
{
}


#include "edit_iface.moc"
