#ifndef __CURSOR_IFACE_IMPL_H__
#define __CURSOR_IFACE_IMPL_H__


#include "keditor/cursor_iface.h"


class KWrite;


class CursorIfaceImpl : public KEditor::CursorDocumentIface
{
  Q_OBJECT

public:

  CursorIfaceImpl(KWrite *edit, KEditor::Document *parent, KEditor::Editor *editor);

  virtual void getCursorPosition(int &line, int &col);
  virtual bool setCursorPosition(int line, int col);
  
  virtual int numberOfLines() const;
  virtual int lengthOfLine(int line) const;
  

private:

  KWrite *m_edit;

};


#endif
