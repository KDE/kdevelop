#include <kdebug.h>


#include "editor.h"


#include "KEditorDebugIface.h"


static QCString generateName()
{
  static int id=0;
  QCString name = QString("KEditor::DebugDocumentIface-%1").arg(++id).latin1();
  return name;
}


KEditor::DebugDocumentDCOPIface::DebugDocumentDCOPIface(KEditor::DebugDocumentIface *iface)
  : QObject(iface), DCOPObject(generateName()), m_iface(iface)
{
  connect(m_iface, SIGNAL(breakPointToggled(KEditor::Document *,int)), this, SLOT(forwardBreakPointToggled(KEditor::Document *,int)));
  connect(m_iface, SIGNAL(breakPointEnabledToggled(KEditor::Document *,int)), this, SLOT(forwardBreakPointEnabledToggled(KEditor::Document *,int)));
}


KEditor::DebugDocumentDCOPIface::~DebugDocumentDCOPIface()
{
  kdDebug() << "Deleted KEditor::DebugDocumentDCOPIface" << endl;
}


bool KEditor::DebugDocumentDCOPIface::markExecutionPoint(int line)
{
  return m_iface->markExecutionPoint(line);
}


bool KEditor::DebugDocumentDCOPIface::setBreakPoint(int line, bool enabled, bool pending)
{
  return m_iface->setBreakPoint(line,enabled,pending);
}


bool KEditor::DebugDocumentDCOPIface::unsetBreakPoint(int line)
{
  return m_iface->unsetBreakPoint(line);
}


void KEditor::DebugDocumentDCOPIface::forwardBreakPointToggled(KEditor::Document *doc, int line)
{
  QByteArray data;
  QDataStream arg(data, IO_WriteOnly);

  arg << doc->dcopInterface() << line;
  
  emitDCOPSignal("breakPointToggled(DCOPRef,int)", data);
}


void KEditor::DebugDocumentDCOPIface::forwardBreakPointEnabledToggled(KEditor::Document *doc, int line)
{
  QByteArray data;
  QDataStream arg(data, IO_WriteOnly);

  arg << doc->dcopInterface() << line;

  emitDCOPSignal("breakPointEnabledToggled(DCOPRef,int)", data);
}


