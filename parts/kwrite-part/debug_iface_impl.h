#ifndef __DEBUG_IFACE_IMPL_H__
#define __DEBUG_IFACE_IMPL_H__


#include "keditor/debug_iface.h"


class KWrite;


class DebugIfaceImpl : public KEditor::DebugDocumentIface
{
  Q_OBJECT

public:

  DebugIfaceImpl(KWrite *edit, KEditor::Document *parent, KEditor::Editor *editor);

  virtual bool markExecutionPoint(int line);   
  virtual bool setBreakPoint(int line, bool enabled, bool pending);   
  virtual bool unsetBreakPoint(int line);
  

private slots:

  void slotToggledBreakpoint(int line);
  void slotEnabledBreakpoint(int lint);


private:

  KWrite *m_edit;

};


#endif
