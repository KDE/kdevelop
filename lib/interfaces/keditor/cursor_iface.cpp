#include <qspinbox.h>


#include <kstdaction.h>
#include <kaction.h>


#include "editor.h"
#include "cursor_iface.h"
#include "gotoline.h"


using namespace KEditor;


CursorEditorIface::CursorEditorIface(Editor *parent)
  : EditorInterface(parent)
{
  _goto = KStdAction::gotoLine(this, SLOT(slotGotoLine()), actionCollection(), "go_goto_line");

  connect(parent, SIGNAL(documentAdded()), this, SLOT(documentChanged()));
  connect(parent, SIGNAL(documentRemoved()), this, SLOT(documentChanged()));
  connect(parent, SIGNAL(documentActivated(Document*)), this, SLOT(documentChanged()));

  documentChanged();
}


void CursorEditorIface::documentChanged()
{
  Document *doc = editor()->currentDocument();
  if (!doc)
  {
	_goto->setEnabled(false);
	return;
  }

  _goto->setEnabled(doc->getInterface("KEditor::CursorDocumentIface"));
}


void CursorEditorIface::slotGotoLine()
{
  Document *doc = editor()->currentDocument();
  if (!doc)
    return;

  CursorDocumentIface *iface = (CursorDocumentIface*) doc->getInterface("KEditor::CursorDocumentIface");
  if (!iface)
    return;

  GotoLineDlg dlg(widget(), "goto_dialog", true);

  dlg.LineNumber->setMaxValue(iface->numberOfLines());
  dlg.LineNumber->setFocus();

  int col, line;

  iface->getCursorPosition(line, col);

  dlg.LineNumber->setValue(line);
  
  if (dlg.exec() != QDialog::Accepted)
	return;
  
  iface->setCursorPosition(dlg.LineNumber->value(), 0);
}



CursorDocumentIface::CursorDocumentIface(Document *parent, Editor *editor)
  : DocumentInterface(parent, editor)
{
}


#include "cursor_iface.moc"
