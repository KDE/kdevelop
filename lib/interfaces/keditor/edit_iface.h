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

  virtual QString text() const = 0;
  virtual void setText(const QString &text) = 0;
	  
  virtual void append(const QString &text) = 0;
  virtual bool insertLine(const QString &text, uint line) = 0;
  virtual bool insertAt(const QString &text, uint line, uint col) = 0;

  virtual bool removeLine(uint line) = 0;

  virtual QString line(uint line) const = 0;
  virtual bool setLine(const QString &text, uint line) = 0;


};


}


#endif
