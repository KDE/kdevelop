#ifndef __KEDITOR_DEBUG_IFACE_H__
#define __KEDITOR_DEBUG_IFACE_H__


#include <qlist.h>


#include <dcopobject.h>
#include <dcopref.h>


#include "debug_iface.h"


namespace KEditor
{


class DebugDocumentIface;


class DebugDocumentDCOPIface : public QObject, public DCOPObject
{
  Q_OBJECT
  K_DCOP

public:

  DebugDocumentDCOPIface(DebugDocumentIface *iface);
  ~DebugDocumentDCOPIface();

  
k_dcop:

  bool markExecutionPoint(int line);
  bool setBreakPoint(int line, bool enabled, bool pending);
  bool unsetBreakPoint(int line);
  

private slots:

  void forwardBreakPointToggled(Document *doc, int line);
  void forwardBreakPointEnabledToggled(Document *doc, int line);
  

private:

  DebugDocumentIface *m_iface;

};


}


#endif
