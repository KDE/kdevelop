#include "edit_iface_impl.h"
#include <private/qrichtext_p.h>
#include <kdebug.h>

EditIfaceImpl::EditIfaceImpl(QEditor *edit, KEditor::Document *parent, KEditor::Editor *editor)
  : KEditor::EditDocumentIface(parent, editor), m_edit(edit)
{
    connect( m_edit, SIGNAL(textChanged()), this, SIGNAL(textChanged()) );
    connect( m_edit, SIGNAL(textChanged()), this, SLOT(slotTextChanged()) );
}



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
  m_edit->insertParagraph( text, line );
  return true;
}


bool EditIfaceImpl::insertAt(const QString &text, uint line, uint col)
{
  m_edit->insertAt(text, line, col);
  return true;
}


bool EditIfaceImpl::removeLine(uint line)
{
  m_edit->removeParagraph( line );
  return true;
}


QString EditIfaceImpl::line(uint line,bool replaceTabsWithSpaces) const
{
    if( replaceTabsWithSpaces ){
        kdDebug( 9007 ) << "EditIfaceImpl::line() replaceTabsWithSpaces not implemented yet!!!" << endl;
    }

    return m_edit->textLine( line );
}


bool EditIfaceImpl::setLine(const QString &text, uint line)
{
  if (line >= m_edit->lines())
    return false;

  m_edit->removeParagraph( line );
  m_edit->insertParagraph( text, line );

  return true;
}


void EditIfaceImpl::slotTextChanged()
{
    int parag, index;
    m_edit->getCursorPosition( &parag, &index );
    emit textChanged( document(), parag, index );
}

#include "edit_iface_impl.moc"
