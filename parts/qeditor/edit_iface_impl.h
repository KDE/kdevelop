#ifndef __EDIT_IFACE_IMPL_H__
#define __EDIT_IFACE_IMPL_H__


#include <qeditor.h>


#include "keditor/edit_iface.h"


class EditIfaceImpl : public KEditor::EditDocumentIface
{
  Q_OBJECT

public:

  EditIfaceImpl(QEditor *edit, KEditor::Document *parent, KEditor::Editor *editor);

  virtual QString text() const;
  virtual void setText(const QString &text);

  virtual void append(const QString &text);
  virtual bool insertLine(const QString &text, uint line);
  virtual bool insertAt(const QString &text, uint line, uint col);

  virtual bool removeLine(uint line);

  virtual QString line(uint line,bool replaceTabsWithSpaces=false) const;
  virtual bool setLine(const QString &text, uint line);

private slots:
    void slotTextChanged();


private:

  QEditor *m_edit;

};


#endif
