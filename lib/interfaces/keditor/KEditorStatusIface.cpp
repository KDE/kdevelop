#include <kdebug.h>


#include "editor.h"


#include "KEditorStatusIface.h"


static QCString generateName()
{
  static int id=0;
  QCString name = QString("KEditor::StatusDocumentIface-%1").arg(++id).latin1();
  return name;
}


KEditor::StatusDocumentDCOPIface::StatusDocumentDCOPIface(KEditor::StatusDocumentIface *iface)
  : QObject(iface), DCOPObject(generateName()), m_iface(iface)
{
  connect(m_iface, SIGNAL(statusChanged(KEditor::Document *)), this, SLOT(forwardStatusChanged(KEditor::Document *)));
  connect(m_iface, SIGNAL(message(KEditor::Document *, const QString&)), this, SLOT(forwardMessage(KEditor::Document *, const QString&)));
}


KEditor::StatusDocumentDCOPIface::~StatusDocumentDCOPIface()
{
  kdDebug() << "Deleted KEditor::StatusDocumentDCOPIface" << endl;
}


bool KEditor::StatusDocumentDCOPIface::modified()
{
  return m_iface->modified();
}


QString KEditor::StatusDocumentDCOPIface::status()
{
  return m_iface->status();
}


void KEditor::StatusDocumentDCOPIface::forwardStatusChanged(KEditor::Document *doc)
{
  QByteArray data;
  QDataStream arg(data, IO_WriteOnly);
  
  arg << doc->dcopInterface();
  
  emitDCOPSignal("statusChanged(DCOPRef)", data);
}


void KEditor::StatusDocumentDCOPIface::forwardMessage(KEditor::Document *doc, const QString &text)
{
  QByteArray data;
  QDataStream arg(data, IO_WriteOnly);

  arg << doc->dcopInterface() << text;
  
  emitDCOPSignal("message(DCOPRef,const QString&)", data);
}

#include "KEditorStatusIface.moc"
