#ifndef __EDIT_IFACE_IMPL_H__
#define __EDIT_IFACE_IMPL_H__


#include <qmultilineedit.h>


#include "keditor/edit_iface.h"


class EditIfaceImpl : public KEditor::EditIface
{
  Q_OBJECT

public:

  EditIfaceImpl(QMultiLineEdit *edit, KEditor::Editor *parent);

  virtual QString getText();
  virtual void setText(const QString &text);


private:

  QMultiLineEdit *m_edit;

};


#endif
