#include <kdebug.h>


#include "kwrite/kwview.h"


#include "edit_iface_impl.h"


EditIfaceImpl::EditIfaceImpl(KWrite *edit, KEditor::Document *parent, KEditor::Editor *editor)
  : EditDocumentIface(parent, editor), m_edit(edit)
{
}


QString EditIfaceImpl::text() const
{
  return m_edit->text();
}


void EditIfaceImpl::setText(const QString &text)
{
  // TODO: find a way to set the text so that it is undoable!
  m_edit->setText(text);
}


#include "edit_iface_impl.moc"
