#ifndef __CURSOR_IFACE_IMPL_H__
#define __CURSOR_IFACE_IMPL_H__


#include <qmultilineedit.h>


#include "keditor/cursor_iface.h"
#include "document_impl.h"

class CursorIfaceImpl : public KEditor::CursorDocumentIface
{
  Q_OBJECT

public:

  CursorIfaceImpl(DocumentImpl *parent, KEditor::Editor *editor);

  virtual void getCursorPosition(int &line, int &col);
  virtual bool setCursorPosition(int line, int col);
  
  virtual int numberOfLines() const;
  virtual int lengthOfLine(int line) const;
  

private:

  DocumentImpl *mDocument;

};


#endif
