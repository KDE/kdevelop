#include <kdebug.h>


#include "kwrite/kwview.h"
#include "kwrite/kwdoc.h"


#include "edit_iface_impl.h"


EditIfaceImpl::EditIfaceImpl(KWrite *edit, KEditor::Document *parent, KEditor::Editor *editor)
  : EditDocumentIface(parent, editor), m_edit(edit)
{
  connect(m_edit->doc(),SIGNAL(textChanged()),SIGNAL(textChanged()));
}


QString EditIfaceImpl::text() const
{
  return m_edit->text();
}


void EditIfaceImpl::setText(const QString &text)
{
  m_edit->document()->setText(text);
  m_edit->document()->setModified(true);
  ((KWriteDoc*)m_edit->document())->updateViews();
}


void EditIfaceImpl::append(const QString &text)
{
  m_edit->document()->insertLine(text);
  m_edit->document()->setModified(true);
  ((KWriteDoc*)m_edit->document())->updateViews();
}


bool EditIfaceImpl::insertLine(const QString &text, uint line)
{
  m_edit->document()->insertLine(text, line);
  m_edit->document()->setModified(true);
  ((KWriteDoc*)m_edit->document())->updateViews();
  return true;
}


bool EditIfaceImpl::insertAt(const QString &text, uint line, uint col)
{
  m_edit->document()->insertAt(text, line, col);
  m_edit->document()->setModified(true);
  ((KWriteDoc*)m_edit->document())->updateViews();
  return true;
}


bool EditIfaceImpl::removeLine(uint line)
{
  m_edit->document()->removeLine(line);
  m_edit->document()->setModified(true);
  ((KWriteDoc*)m_edit->document())->updateViews();
  return true;
}


QString EditIfaceImpl::line(uint line) const
{
  return m_edit->document()->textLine(line);
}


bool EditIfaceImpl::setLine(const QString &text, uint line)
{
  if (line >= m_edit->document()->numLines())
    return false;

  m_edit->document()->removeLine(line);
  m_edit->document()->insertLine(text, line);
  m_edit->document()->setModified(true);
  ((KWriteDoc*)m_edit->document())->updateViews();

  return true;
}

bool EditIfaceImpl::hasSelectedText()
{
	return (( KWriteDoc* ) m_edit->document())->hasMarkedText();
}



#include "edit_iface_impl.moc"
