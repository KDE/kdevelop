
#include "formwindow_page.h"
#include "guibuilder_part.h"

FormWindowPage::FormWindowPage(GuiBuilderPart *guibuilder, QWidget *parent)
  : QWidget(parent), m_guibuilder(guibuilder)
{
}

FormWindowPage::~FormWindowPage()
{
}

GuiBuilderPart *FormWindowPage::guibuilder() const
{
  return m_guibuilder;
}
  
#include "formwindow_page.moc"
