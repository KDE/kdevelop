#include <kstdaction.h>
#include <kaction.h>
#include <kdebug.h>


#include "editor.h"
#include "undo_iface.h"


using namespace KEditor;


UndoDocumentIface::UndoDocumentIface(Document *parent, Editor *editor)
  : DocumentInterface(parent, editor)
{
  _undoAction = KStdAction::undo(this, SLOT(slotUndo()), parent->actionCollection(), "edit_undo");
  _redoAction = KStdAction::redo(this, SLOT(slotRedo()), parent->actionCollection(), "edit_redo");

  connect(this, SIGNAL(undoAvailable(bool)), this, SLOT(undoChanged()));
  connect(this, SIGNAL(redoAvailable(bool)), this, SLOT(undoChanged()));

  _undoAction->setEnabled(false);
  _redoAction->setEnabled(false);
}


void UndoDocumentIface::undoChanged()
{
  _undoAction->setEnabled(undoAvailable());
  _redoAction->setEnabled(redoAvailable());
}


void UndoDocumentIface::slotUndo()
{
  (void) undo();
}


void UndoDocumentIface::slotRedo()
{
  (void) redo();
}


#include "undo_iface.moc"
