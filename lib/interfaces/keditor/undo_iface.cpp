#include <kstdaction.h>
#include <kaction.h>
#include <kdebug.h>


#include "editor.h"


#include "undo_iface.h"
#include "KEditorUndoIface.h"


KEditor::UndoDocumentIface::UndoDocumentIface(Document *parent, Editor *editor)
  : KEditor::DocumentInterface(parent, editor)
{
  _undoAction = KStdAction::undo(this, SLOT(slotUndo()), parent->actionCollection(), "edit_undo");
  _redoAction = KStdAction::redo(this, SLOT(slotRedo()), parent->actionCollection(), "edit_redo");

  connect(this, SIGNAL(undoAvailable(KEditor::Document*,bool)), this, SLOT(undoChanged()));
  connect(this, SIGNAL(redoAvailable(KEditor::Document*,bool)), this, SLOT(undoChanged()));

  _undoAction->setEnabled(false);
  _redoAction->setEnabled(false);

  m_dcopIface = new KEditor::UndoDocumentDCOPIface(this);
}


DCOPRef KEditor::UndoDocumentIface::dcopInterface() const
{
  return DCOPRef(m_dcopIface);
}


void KEditor::UndoDocumentIface::undoChanged()
{
  _undoAction->setEnabled(undoAvailable());
  _redoAction->setEnabled(redoAvailable());
}


void KEditor::UndoDocumentIface::slotUndo()
{
  (void) undo();
}


void KEditor::UndoDocumentIface::slotRedo()
{
  (void) redo();
}


KEditor::UndoDocumentIface *KEditor::UndoDocumentIface::interface(KEditor::Document *doc)
{
  return static_cast<KEditor::UndoDocumentIface*>(doc->queryInterface("KEditor::UndoDocumentIface"));
}
