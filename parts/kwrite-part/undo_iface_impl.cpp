#include "kwrite/kwview.h"


#include "undo_iface_impl.h"


using namespace KEditor;


UndoIfaceImpl::UndoIfaceImpl(KWrite *edit, Document *parent, Editor *editor)
  : UndoDocumentIface(parent, editor), m_edit(edit), _undo(false), _redo(false)
{
  connect(m_edit, SIGNAL(newUndo()), this, SLOT(slotUndoStatus()));
}


bool UndoIfaceImpl::undo()
{
  m_edit->undo();
  return true;
}


bool UndoIfaceImpl::redo()
{
  m_edit->redo();
  return true;
}


bool UndoIfaceImpl::undoAvailable()
{
  return _undo;
}


bool UndoIfaceImpl::redoAvailable()
{
  return _redo;
}


void UndoIfaceImpl::slotUndoStatus()
{
  int state = m_edit->undoState();
  _undo = state & 1;
  emit UndoDocumentIface::undoAvailable(_undo);
  _redo = state & 2;
  emit UndoDocumentIface::redoAvailable(_redo);
}


#include "undo_iface_impl.moc"
