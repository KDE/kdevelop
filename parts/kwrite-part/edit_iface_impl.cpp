#include <kdebug.h>


#include "kwrite/kwview.h"
#include "kwrite/kwdoc.h"


#include "edit_iface_impl.h"


EditIfaceImpl::EditIfaceImpl(KWrite *edit, KEditor::Document *parent, KEditor::Editor *editor)
  : EditDocumentIface(parent, editor), m_edit(edit)
{
  connect(m_edit->doc(),SIGNAL(textChanged()),SLOT(slotTextChanged()));
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


QString EditIfaceImpl::line(uint line,bool replaceTabsWithSpaces) const
{
  if(replaceTabsWithSpaces){
    QString lineStr = m_edit->document()->textLine(line);
    QString spaces;
    spaces.fill(' ',((KWriteDoc*)m_edit->document())->tabWidth());
    return lineStr.replace(QRegExp("\t"),spaces);		 
    
  }
  else {
    return m_edit->document()->textLine(line);
  }
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

void EditIfaceImpl::slotTextChanged()
{
	emit EditDocumentIface::textChanged( document(), m_edit->currentLine(), m_edit->currentColumn());
	emit EditDocumentIface::textChanged();
}

#include "edit_iface_impl.moc"
