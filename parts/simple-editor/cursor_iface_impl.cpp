#include <qfile.h>
#include <qtextstream.h>


#include "cursor_iface_impl.h"


using namespace KEditor;


CursorIfaceImpl::CursorIfaceImpl(QMultiLineEdit *edit, Document *parent, Editor *editor)
  : CursorDocumentIface(parent, editor), m_edit(edit)
{
}


bool CursorIfaceImpl::setCursorPosition(int line, int col)
{
  m_edit->setCursorPosition(line, col);
  m_edit->setFocus();

  return true;
}


int CursorIfaceImpl::numberOfLines()
{
  return m_edit->numLines();
}


int CursorIfaceImpl::lengthOfLine(int line)
{
  return m_edit->textLine(line).length();
}


void CursorIfaceImpl::getCursorPosition(int &line, int &col)
{
  m_edit->getCursorPosition(&line, &col);
}


#include "cursor_iface_impl.moc"
