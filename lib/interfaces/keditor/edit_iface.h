#ifndef __EDIT_IFACE_H__
#define __EDIT_IFACE_H__


#include <qstring.h>


#include "interface.h"


namespace KEditor {


class EditIface : public KEditor::DocumentInterface
{
  Q_OBJECT

public:

  EditIface(Editor *parent);

  virtual QString getText() = 0;
  virtual void setText(const QString &text) = 0;
	  
  virtual void append(QString text) = 0;
  virtual bool insertLine(QString text, uint line) = 0;
  virtual bool insertAt(QString text, uint line, uint col) = 0;

  virtual bool removeLine(uint line) = 0;

  virtual QString getLine(uint line) = 0;
  virtual bool setLine(QString text, uint line) = 0;


};


}


#endif
