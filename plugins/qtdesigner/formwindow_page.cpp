
#include "formwindow_page.h"
#include "qtdesignerpart.h"

FormWindowPage::FormWindowPage(QtDesignerPart *guibuilder, QWidget *parent)
  : QWidget(parent), m_guibuilder(guibuilder)
{
}

FormWindowPage::~FormWindowPage()
{
}

QtDesignerPart *FormWindowPage::guibuilder() const
{
  return m_guibuilder;
}
  
#include "formwindow_page.moc"
