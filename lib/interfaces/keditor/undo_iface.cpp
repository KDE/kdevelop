#include <kstdaction.h>
#include <kaction.h>
#include <kdebug.h>


#include "editor.h"
#include "undo_iface.h"


using namespace KEditor;


UndoDocumentIface::UndoDocumentIface(Document *parent, Editor *editor)
  : DocumentInterface(parent, editor)
{
  UndoEditorIface *iface = (UndoEditorIface*)editor->getInterface("KEditor::UndoEditorIface");
  if (iface)
  {
    connect(this, SIGNAL(undoAvailable(bool)), iface, SLOT(undoChanged()));
	connect(this, SIGNAL(redoAvailable(bool)), iface, SLOT(undoChanged()));
  }
}




UndoEditorIface::UndoEditorIface(Editor *parent)
  : EditorInterface(parent)
{
  _undoAction = KStdAction::undo(this, SLOT(slotUndo()), actionCollection(), "edit_undo");
  _redoAction = KStdAction::redo(this, SLOT(slotRedo()), actionCollection(), "edit_redo");

  connect(parent, SIGNAL(documentAdded()), this, SLOT(undoChanged()));
  connect(parent, SIGNAL(documentRemoved()), this, SLOT(undoChanged()));
  connect(parent, SIGNAL(documentActivated(Document*)), this, SLOT(undoChanged()));
	  
  undoChanged();
}


void UndoEditorIface::undoChanged()
{
  UndoDocumentIface *iface = documentIface();
  if (!iface)
  {
    _undoAction->setEnabled(false);
    _redoAction->setEnabled(false);
	return;
  }

  _undoAction->setEnabled(iface->undoAvailable());
  _redoAction->setEnabled(iface->redoAvailable());
}


void UndoEditorIface::slotUndo()
{
  UndoDocumentIface *iface = documentIface();
  if (iface)
    iface->undo();
}


void UndoEditorIface::slotRedo()
{
  UndoDocumentIface *iface = documentIface();
  if (iface)
    iface->redo();
}


UndoDocumentIface *UndoEditorIface::documentIface()
{
  Document *doc = editor()->currentDocument();
  if (!doc)
	return 0;

  return (UndoDocumentIface*) doc->getInterface("KEditor::UndoDocumentIface");
}


#include "undo_iface.moc"
