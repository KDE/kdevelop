#include <kstdaction.h>
#include <kaction.h>
#include <kdebug.h>


#include <qclipboard.h>
#include <qapplication.h>


#include "editor.h"


#include "clipboard_iface.h"


KEditor::ClipboardDocumentIface::ClipboardDocumentIface(Document *parent, Editor *editor)  : KEditor::DocumentInterface(parent, editor)
{
  _cutAction = KStdAction::cut(this, SLOT(slotCut()), parent->actionCollection(), "edit_cut");
  _copyAction = KStdAction::copy(this, SLOT(slotCopy()), parent->actionCollection(), "edit_copy");
  _pasteAction = KStdAction::paste(this, SLOT(slotPaste()), parent->actionCollection(), "edit_paste");

  _cutAction->setEnabled(false);
  _copyAction->setEnabled(false);

  connect(QApplication::clipboard(), SIGNAL(dataChanged()), this, SLOT(clipboardChanged()));
  connect(this, SIGNAL(copyAvailable(KEditor::Document*,bool)), this, SLOT(clipboardChanged()));

  // set the initial state of the paste action
  //
  // Note: it would sound reasonable to call 'clipboardChanged'
  // here, but as we are in the constructor, and clipboardChanged
  // calls abstract functions, this would fail.
  QString text = QApplication::clipboard()->text();
  _pasteAction->setEnabled(!text.isEmpty());
}


void KEditor::ClipboardDocumentIface::clipboardChanged()
{
  QString text = QApplication::clipboard()->text();
  _pasteAction->setEnabled(!text.isEmpty());

  _cutAction->setEnabled(copyAvailable());
  _copyAction->setEnabled(copyAvailable());
}


void KEditor::ClipboardDocumentIface::slotCut()
{
  (void) cut();
}


void KEditor::ClipboardDocumentIface::slotCopy()
{
  (void) copy();
}


void KEditor::ClipboardDocumentIface::slotPaste()
{
  (void) paste();
}


KEditor::ClipboardDocumentIface *KEditor::ClipboardDocumentIface::interface(KEditor::Document *doc)
{
  return static_cast<KEditor::ClipboardDocumentIface*>(doc->queryInterface("KEditor::ClipboardDocumentIface"));
}


#include "clipboard_iface.moc"
