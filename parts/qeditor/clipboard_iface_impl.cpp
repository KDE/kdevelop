#include <kdebug.h>


#include "clipboard_iface_impl.h"


using namespace KEditor;


ClipboardIfaceImpl::ClipboardIfaceImpl(QEditor *edit, Document *parent, Editor *editor)
  : ClipboardDocumentIface(parent, editor), m_edit(edit), _available(false)
{
  connect(m_edit, SIGNAL(copyAvailable(bool)), this, SLOT(slotCopyAvailable(bool)));
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


void ClipboardIfaceImpl::slotCopyAvailable(bool avail)
{
  _available = avail;
  emit ClipboardDocumentIface::copyAvailable(document(), avail);
}

#include "clipboard_iface_impl.moc"
