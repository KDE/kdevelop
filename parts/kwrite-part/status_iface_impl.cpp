#include <kdebug.h>
#include <klocale.h>


#include "kwrite/kwview.h"


#include "keditor/editor.h"
#include "status_iface_impl.h"


StatusIfaceImpl::StatusIfaceImpl(KWrite *edit, KEditor::Document *parent, KEditor::Editor *editor)
  : StatusDocumentIface(parent, editor), m_edit(edit)
{
  connect(edit, SIGNAL(newStatus()), this, SLOT(slotStatusChanged()));
  connect(edit, SIGNAL(statusMsg(const QString &)), this, SLOT(slotMessage(const QString &)));
}


bool StatusIfaceImpl::modified()
{
  document()->setModified(m_edit->isModified());

  return m_edit->isModified();
}


QString StatusIfaceImpl::status()
{
  QString s;
  if (m_edit->isOverwriteMode())
    s = i18n("OVR");
  else
    s = i18n("INS");
  if (m_edit->isReadOnly())
    s += i18n(", ro");
  return s;
}


void StatusIfaceImpl::slotStatusChanged()
{
  emit StatusDocumentIface::statusChanged(document());
}


void StatusIfaceImpl::slotMessage(const QString &text)
{
  emit StatusDocumentIface::message(document(), text);
}


#include "status_iface_impl.moc"
