#include "edit_iface_impl.h"


using namespace KEditor;


EditIfaceImpl::EditIfaceImpl(QMultiLineEdit *edit, Editor *parent)
    : EditIface(parent), m_edit(edit)
{}



QString EditIfaceImpl::getText()
{
  return m_edit->text();
}


void EditIfaceImpl::setText(const QString &text)
{
  m_edit->setText(text);
}


#include "edit_iface_impl.moc"
