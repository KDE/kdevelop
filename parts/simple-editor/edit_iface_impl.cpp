#include "edit_iface_impl.h"


EditIfaceImpl::EditIfaceImpl(QMultiLineEdit *edit, KEditor::Document *parent, KEditor::Editor *editor)
  : KEditor::EditDocumentIface(parent, editor), m_edit(edit)
{}



QString EditIfaceImpl::text() const
{
  return m_edit->text();
}


void EditIfaceImpl::setText(const QString &text)
{
  m_edit->setText(text);
}


void EditIfaceImpl::append(const QString &text)
{
  m_edit->append(text);
}


bool EditIfaceImpl::insertLine(const QString &text, uint line)
{
  m_edit->insertLine(text, line);
  return true;
}


bool EditIfaceImpl::insertAt(const QString &text, uint line, uint col)
{
  m_edit->insertAt(text, line, col);
  return true;
}


bool EditIfaceImpl::removeLine(uint line)
{
  m_edit->removeLine(line);
  return true;
}


QString EditIfaceImpl::line(uint line,bool replaceTabsWithSpaces) const
{
  return m_edit->textLine(line);
}


bool EditIfaceImpl::setLine(const QString &text, uint line)
{
  if (line >= m_edit->numLines())
    return false;

  m_edit->removeLine(line);
  m_edit->insertLine(text, line);

  return true;
}


#include "edit_iface_impl.moc"
