#include "undo_iface_impl.h"


using namespace KEditor;


UndoIfaceImpl::UndoIfaceImpl(QMultiLineEdit *edit, Document *parent, Editor *editor)
  : UndoDocumentIface(parent, editor), m_edit(edit), _undo(false), _redo(false)
{
  m_edit->setUndoRedoEnabled(true);

  connect(m_edit, SIGNAL(undoAvailable(bool)), this, SLOT(slotUndoAvailable(bool)));
  connect(m_edit, SIGNAL(redoAvailable(bool)), this, SLOT(slotRedoAvailable(bool)));
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


void UndoIfaceImpl::slotUndoAvailable(bool avail)
{
  _undo = avail;
  emit UndoDocumentIface::undoAvailable(document(), avail);
}


void UndoIfaceImpl::slotRedoAvailable(bool avail)
{
  _redo = avail;
  emit UndoDocumentIface::redoAvailable(document(), avail);
}


#include "undo_iface_impl.moc"
