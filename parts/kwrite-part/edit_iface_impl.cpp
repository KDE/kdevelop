#include <kdebug.h>

#include "kwrite/kwview.h"
#include "kwrite/kwdoc.h"


#include "edit_iface_impl.h"


EditIfaceImpl::EditIfaceImpl(KWrite *edit, KEditor::Document *parent, KEditor::Editor *editor)
  : EditDocumentIface(parent, editor), m_edit(edit)
{
  connect(m_edit->document(),SIGNAL(textChanged()),SLOT(slotTextChanged()));
}


QString EditIfaceImpl::text() const
{
  return m_edit->text();
}


void EditIfaceImpl::setText(const QString &text)
{
  KWriteDoc* doc = (KWriteDoc*)m_edit->document()->qt_cast( "KWriteDoc" );
  if ( !doc )
    return;
  doc->setText(text);
  doc->setModified(true);
  doc->updateViews();
}


void EditIfaceImpl::append(const QString &text)
{
  KWriteDoc* doc = (KWriteDoc*)m_edit->document()->qt_cast( "KWriteDoc" );
  if ( !doc )
    return;
  doc->insertLine(text);
  doc->setModified(true);
  doc->updateViews();
}


bool EditIfaceImpl::insertLine(const QString &text, uint line)
{
  KWriteDoc* doc = (KWriteDoc*)m_edit->document()->qt_cast( "KWriteDoc" );
  if ( !doc )
    return false;
  doc->insertLine(text, line);
  doc->setModified(true);
  doc->updateViews();
  return true;
}


bool EditIfaceImpl::insertAt(const QString &text, uint line, uint col)
{
  KWriteDoc* doc = (KWriteDoc*)m_edit->document()->qt_cast( "KWriteDoc" );
  if ( !doc )
    return false;
  doc->insertAt(text, line, col);
  doc->setModified(true);
  doc->updateViews();
  return true;
}


bool EditIfaceImpl::removeLine(uint line)
{
  KWriteDoc* doc = (KWriteDoc*)m_edit->document()->qt_cast( "KWriteDoc" );
  if ( !doc )
    return false;
  doc->removeLine(line);
  doc->setModified(true);
  doc->updateViews();
  return true;
}


QString EditIfaceImpl::line(uint line,bool replaceTabsWithSpaces) const
{
  KWriteDoc* doc = (KWriteDoc*)m_edit->document()->qt_cast( "KWriteDoc" );
  if ( !doc )
    return QString::null;
  if(replaceTabsWithSpaces){
    QString lineStr = doc->textLine(line);
    QString spaces;
    spaces.fill(' ',doc->tabWidth());
    return lineStr.replace(QRegExp("\t"),spaces);

  }
  else {
    return doc->textLine(line);
  }
}


bool EditIfaceImpl::setLine(const QString &text, uint line)
{
    KWriteDoc* doc = (KWriteDoc*)m_edit->document()->qt_cast( "KWriteDoc" );
  if ( !doc )
    return false;
  if (line >= doc->numLines())
    return false;

  doc->removeLine(line);
  doc->insertLine(text, line);
  doc->setModified(true);
  doc->updateViews();

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
