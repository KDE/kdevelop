#ifndef __CURSOR_IFACE_H__
#define __CURSOR_IFACE_H__


#include "interface.h"


class KAction;


namespace KEditor {


class CursorDocumentIface : public KEditor::DocumentInterface
{
  Q_OBJECT

public:

  CursorDocumentIface(Document *parent, Editor *editor);

  virtual bool setCursorPosition(int line, int row) = 0;
  virtual void getCursorPosition(int &line, int &col) = 0;

  virtual int numberOfLines() = 0;
  virtual int lengthOfLine(int line) = 0;


signals:

  void cursorPositionChanged(int line, int col);


private slots:
	   
  void slotGotoLine();


};


}


#endif
