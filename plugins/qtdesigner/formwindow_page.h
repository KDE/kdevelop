#ifndef FORMWINDOW_PAGE_H
#define FORMWINDOW_PAGE_H

#include <QtGui/QWidget>

class QtDesignerPart;

class FormWindowPage: public QWidget
{
  Q_OBJECT
public:
  FormWindowPage(QtDesignerPart *guibuilder, QWidget *parent = 0);
  virtual ~FormWindowPage();

  QtDesignerPart *guibuilder() const;

private:
  QtDesignerPart *m_guibuilder;
};

#endif // FORMWINDOW_PAGE_H

