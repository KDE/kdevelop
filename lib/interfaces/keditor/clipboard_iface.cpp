#include <kstdaction.h>
#include <kaction.h>
#include <kdebug.h>


#include <qclipboard.h>
#include <qapplication.h>


#include "editor.h"


#include "clipboard_iface.h"


using namespace KEditor;


ClipboardEditorIface::ClipboardEditorIface(Editor *parent)
  : EditorInterface(parent)
{
  _cutAction = KStdAction::cut(this, SLOT(slotCut()), actionCollection(), "edit_cut");
  _copyAction = KStdAction::copy(this, SLOT(slotCopy()), actionCollection(), "edit_copy");
  _pasteAction = KStdAction::paste(this, SLOT(slotPaste()), actionCollection(), "edit_paste");

  _cutAction->setEnabled(false);
  _copyAction->setEnabled(false);

  connect(QApplication::clipboard(), SIGNAL(dataChanged()), this, SLOT(clipboardChanged()));
  connect(parent, SIGNAL(documentAdded()), this, SLOT(clipboardChanged()));
  connect(parent, SIGNAL(documentRemoved()), this, SLOT(clipboardChanged()));
  connect(parent, SIGNAL(documentActivated(Document*)), this, SLOT(clipboardChanged()));

  clipboardChanged();
}


void ClipboardEditorIface::clipboardChanged()
{
kdDebug() << "CLIPBOARD changed" << endl;

  ClipboardDocumentIface *iface = documentIface();
kdDebug() << "  iface: " << iface << endl;  
  if (!iface)
	{
	  _pasteAction->setEnabled(false);
	  _cutAction->setEnabled(false);
	  _copyAction->setEnabled(false);
	  return;
	}
  
  QString text = QApplication::clipboard()->text();
  _pasteAction->setEnabled(!text.isEmpty());

  _cutAction->setEnabled(iface->copyAvailable());
  _copyAction->setEnabled(iface->copyAvailable());  
}


void ClipboardEditorIface::slotCut()
{
  ClipboardDocumentIface *iface = documentIface();   
  if (!iface)     
	return;  
 
  (void) iface->cut();
}


void ClipboardEditorIface::slotCopy()
{
  ClipboardDocumentIface *iface = documentIface();
  if (!iface)
	return;

  (void) iface->copy();
}


void ClipboardEditorIface::slotPaste()
{
  ClipboardDocumentIface *iface = documentIface();
  if (!iface)
    return;

  (void) iface->paste();
}


ClipboardDocumentIface *ClipboardEditorIface::documentIface()
{
  Document *doc = editor()->currentDocument();
  if (!doc)
	return 0;

  return (ClipboardDocumentIface*) doc->getInterface("KEditor::ClipboardDocumentIface");
}



ClipboardDocumentIface::ClipboardDocumentIface(Document *parent, Editor *editor)
  : DocumentInterface(parent, editor)
{
  ClipboardEditorIface *iface = (ClipboardEditorIface*)editor->getInterface("KEditor::ClipboardEditorIface");
  if (iface)
    connect(this, SIGNAL(copyAvailable(bool)), iface, SLOT(clipboardChanged()));
}


#include "clipboard_iface.moc"
