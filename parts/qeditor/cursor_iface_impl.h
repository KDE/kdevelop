#ifndef __CURSOR_IFACE_IMPL_H__
#define __CURSOR_IFACE_IMPL_H__


#include <qeditor.h>


#include "keditor/cursor_iface.h"


class CursorIfaceImpl : public KEditor::CursorDocumentIface
{
  Q_OBJECT

public:

  CursorIfaceImpl(QEditor *edit, KEditor::Document *parent, KEditor::Editor *editor);

  virtual void getCursorPosition(int &line, int &col);
  virtual bool setCursorPosition(int line, int col);

  virtual int numberOfLines() const;
  virtual int lengthOfLine(int line) const;


private:

  QEditor *m_edit;

};


#endif
