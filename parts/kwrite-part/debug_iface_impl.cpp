#include <kdebug.h>


#include "kwrite/kwview.h"
#include "kwrite/kwdoc.h"


#include "debug_iface_impl.h"


DebugIfaceImpl::DebugIfaceImpl(KWrite *edit, KEditor::Document *parent, KEditor::Editor *editor)
  : DebugDocumentIface(parent, editor), m_edit(edit)
{
  connect(edit, SIGNAL(toggledBreakpoint(int)), this, SLOT(slotToggledBreakpoint(int)));
  connect(edit, SIGNAL(editedBreakpoint(int)), this, SLOT(slotToggledBreakpoint(int)));
  connect(edit, SIGNAL(toggledBreakpointEnabled(int)), this, SLOT(slotEnabledBreakpoint(int)));
}


bool DebugIfaceImpl::markExecutionPoint(int line)
{
  KWriteDoc* doc=dynamic_cast<KWriteDoc*>(m_edit->document());
  doc->setExecutionPoint(line);
  return true;
}


bool DebugIfaceImpl::setBreakPoint(int line, bool enabled, bool pending)
{
  KWriteDoc* doc=dynamic_cast<KWriteDoc*>(m_edit->document());
  doc->setBreakpoint(line, 0, enabled, pending);
  return true;
}


bool DebugIfaceImpl::unsetBreakPoint(int line)
{
  KWriteDoc* doc=dynamic_cast<KWriteDoc*>(m_edit->document());
  doc->setBreakpoint(line, -1, false, false);
  return true;
}


void DebugIfaceImpl::slotToggledBreakpoint(int line)
{
  emit DebugDocumentIface::breakPointToggled(document(), line);
}


void DebugIfaceImpl::slotEnabledBreakpoint(int line)
{
  emit DebugDocumentIface::breakPointEnabledToggled(document(), line);
}


#include "debug_iface_impl.moc"
