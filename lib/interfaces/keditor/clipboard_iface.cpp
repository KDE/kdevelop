#include <kstdaction.h>
#include <kaction.h>
#include <kdebug.h>


#include <qclipboard.h>
#include <qapplication.h>


#include "editor.h"


#include "clipboard_iface.h"


using namespace KEditor;


ClipboardDocumentIface::ClipboardDocumentIface(Document *parent, Editor *editor)  : DocumentInterface(parent, editor)
{
  _cutAction = KStdAction::cut(this, SLOT(slotCut()), parent->actionCollection(), "edit_cut");
  _copyAction = KStdAction::copy(this, SLOT(slotCopy()), parent->actionCollection(), "edit_copy");
  _pasteAction = KStdAction::paste(this, SLOT(slotPaste()), parent->actionCollection(), "edit_paste");

  _cutAction->setEnabled(false);
  _copyAction->setEnabled(false);

  connect(QApplication::clipboard(), SIGNAL(dataChanged()), this, SLOT(clipboardChanged()));
  connect(this, SIGNAL(copyAvailable(bool)), this, SLOT(clipboardChanged()));

  // set the initial state of the paste action
  //
  // Note: it would sound reasonable to call 'clipboardChanged'
  // here, but as we are in the constructor, and clipboardChanged
  // calls abstract functions, this would fail.
  QString text = QApplication::clipboard()->text();
  _pasteAction->setEnabled(!text.isEmpty());
}


void ClipboardDocumentIface::clipboardChanged()
{
  QString text = QApplication::clipboard()->text();
  _pasteAction->setEnabled(!text.isEmpty());

  _cutAction->setEnabled(copyAvailable());
  _copyAction->setEnabled(copyAvailable());
}


void ClipboardDocumentIface::slotCut()
{
  (void) cut();
}


void ClipboardDocumentIface::slotCopy()
{
  (void) copy();
}


void ClipboardDocumentIface::slotPaste()
{
  (void) paste();
}


#include "clipboard_iface.moc"
