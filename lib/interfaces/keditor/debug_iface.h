#ifndef __DEBUG_IFACE_H__
#define __DEBUG_IFACE_H__


#include "interface.h"


namespace KEditor {
        

class DebugDocumentIface : public KEditor::DocumentInterface
{
  Q_OBJECT

public:

  DebugDocumentIface(Document *parent, Editor *editor);

  virtual bool markExecutionPoint(int line) = 0;
  virtual bool setBreakPoint(int line, bool enabled, bool pending) = 0;
  virtual bool unsetBreakPoint(int line) = 0;

  static DebugDocumentIface *interface(Document *doc);


signals:

  void breakPointToggled(KEditor::Document *doc, int line);
  void breakPointEnabledToggled(KEditor::Document *doc, int line);

};


}


#endif
