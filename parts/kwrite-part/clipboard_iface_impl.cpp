#include <kdebug.h>


#include "kwrite/kwview.h"


#include "clipboard_iface_impl.h"


ClipboardIfaceImpl::ClipboardIfaceImpl(KWrite *edit, KEditor::Document *parent, KEditor::Editor *editor)
  : ClipboardDocumentIface(parent, editor), m_edit(edit), _available(false)
{
  connect(m_edit, SIGNAL(newMarkStatus()), this, SLOT(slotCopyAvailable()));
}


bool ClipboardIfaceImpl::cut()
{
  m_edit->cut();

  return true;
}


bool ClipboardIfaceImpl::copy()
{
  m_edit->copy();

  return true;
}


bool ClipboardIfaceImpl::paste()
{
  m_edit->paste();

  return true;
}


bool ClipboardIfaceImpl::copyAvailable()
{
  return _available;
}


void ClipboardIfaceImpl::slotCopyAvailable()
{
  _available = m_edit->hasMarkedText();
  emit ClipboardDocumentIface::copyAvailable(document(), _available);
}


#include "clipboard_iface_impl.moc"
