#ifndef __DEBUG_IFACE_H__
#define __DEBUG_IFACE_H__


#include "interface.h"


namespace KEditor {


class DebugDocumentDCOPIface;


class DebugDocumentIface : public KEditor::DocumentInterface
{
  Q_OBJECT

public:

  DebugDocumentIface(Document *parent, Editor *editor);

  virtual bool markExecutionPoint(int line) = 0;
  virtual bool setBreakPoint(int line, bool enabled, bool pending) = 0;
  virtual bool unsetBreakPoint(int line) = 0;

  virtual DCOPRef dcopInterface() const;

  static DebugDocumentIface *interface(Document *doc);


signals:

  void breakPointToggled(KEditor::Document *doc, int line);
  void breakPointEnabledToggled(KEditor::Document *doc, int line);


private:

  DebugDocumentDCOPIface *m_dcopIface;
  
};


}


#endif
