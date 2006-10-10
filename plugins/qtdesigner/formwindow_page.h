#ifndef FORMWINDOW_PAGE_H
#define FORMWINDOW_PAGE_H

#include <QtGui/QWidget>

class GuiBuilderPart;

class FormWindowPage: public QWidget
{
  Q_OBJECT
public:
  FormWindowPage(GuiBuilderPart *guibuilder, QWidget *parent = 0);
  virtual ~FormWindowPage();

  GuiBuilderPart *guibuilder() const;

private:
  GuiBuilderPart *m_guibuilder;
};

#endif // FORMWINDOW_PAGE_H
