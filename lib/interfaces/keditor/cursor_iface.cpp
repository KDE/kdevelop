#include <qspinbox.h>


#include <kstdaction.h>
#include <kaction.h>


#include "editor.h"
#include "cursor_iface.h"
#include "gotoline.h"


KEditor::CursorDocumentIface::CursorDocumentIface(Document *parent, Editor *editor)
  : KEditor::DocumentInterface(parent, editor)
{
  KStdAction::gotoLine(this, SLOT(slotGotoLine()), parent->actionCollection(), "go_goto_line");
}


void KEditor::CursorDocumentIface::slotGotoLine()
{
  GotoLineDlg dlg(0, "goto_dialog", true);

  dlg.LineNumber->setMaxValue(numberOfLines());
  dlg.LineNumber->setFocus();

  int col, line;

  getCursorPosition(line, col);

  dlg.LineNumber->setValue(line);
  
  if (dlg.exec() != QDialog::Accepted)
    return;
  
  setCursorPosition(dlg.LineNumber->value(), 0);
}


KEditor::CursorDocumentIface *KEditor::CursorDocumentIface::interface(KEditor::Document *doc)
{
  return static_cast<KEditor::CursorDocumentIface*>(doc->queryInterface("KEditor::CursorDocumentIface"));
}


#include "cursor_iface.moc"
