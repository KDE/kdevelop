#ifndef __EDIT_IFACE_IMPL_H__
#define __EDIT_IFACE_IMPL_H__


#include <qmultilineedit.h>


#include "keditor/edit_iface.h"


class EditIfaceImpl : public KEditor::EditDocumentIface
{
  Q_OBJECT

public:

  EditIfaceImpl(KWrite *edit, KEditor::Document *parent, KEditor::Editor *editor);

  virtual QString text() const;
  virtual void setText(const QString &text);

  virtual void append(const QString &text);
  virtual bool insertLine(const QString &text, uint line);
  virtual bool insertAt(const QString &text, uint line, uint col);
         
  virtual bool removeLine(uint line);
           
  virtual QString line(uint line) const;
  virtual bool setLine(const QString &text, uint line);
  

private:

  KWrite *m_edit;

};


#endif
