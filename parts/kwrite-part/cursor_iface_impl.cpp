#include <kdebug.h>


#include "kwrite/kwview.h"


#include "cursor_iface_impl.h"


CursorIfaceImpl::CursorIfaceImpl(KWrite *edit, KEditor::Document *parent, KEditor::Editor *editor)
  : CursorDocumentIface(parent, editor), m_edit(edit)
{
  connect(edit, SIGNAL(newCurPos()), this, SLOT(slotCursorChanged()));
}


bool CursorIfaceImpl::setCursorPosition(int line, int col)
{
  m_edit->setCursorPosition(line, col);
  m_edit->setFocus();

  return true;
}


int CursorIfaceImpl::numberOfLines() const
{
  return m_edit->numLines();
}


int CursorIfaceImpl::lengthOfLine(int line) const
{
  return m_edit->textLine(line).length();
}


void CursorIfaceImpl::getCursorPosition(int &line, int &col)
{
  m_edit->getCursorPosition(&line, &col);
}


void CursorIfaceImpl::slotCursorChanged()
{
  emit CursorDocumentIface::cursorPositionChanged(document(), m_edit->currentLine(), m_edit->currentColumn());
}


#include "cursor_iface_impl.moc"
