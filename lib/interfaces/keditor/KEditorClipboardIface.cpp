#include <kdebug.h>


#include "editor.h"


#include "KEditorClipboardIface.h"


static QCString generateName()
{
  static int id=0;
  QCString name = QString("KEditor::ClipboardDocumentIface-%1").arg(++id).latin1();
  return name;
}


KEditor::ClipboardDocumentDCOPIface::ClipboardDocumentDCOPIface(KEditor::ClipboardDocumentIface *iface)
  : QObject(iface), DCOPObject(generateName()), m_iface(iface)
{
  connect(m_iface, SIGNAL(copyAvailable(KEditor::Document *, bool)), this, SLOT(forwardCopyAvailable(KEditor::Document *, bool)));
}


KEditor::ClipboardDocumentDCOPIface::~ClipboardDocumentDCOPIface()
{
  kdDebug() << "Deleted KEditor::ClipboardDocumentDCOPIface" << endl;
}


void KEditor::ClipboardDocumentDCOPIface::cut()
{
  m_iface->cut();
}


void KEditor::ClipboardDocumentDCOPIface::copy()
{
  m_iface->copy();
}


void KEditor::ClipboardDocumentDCOPIface::paste()
{
  m_iface->paste();
}


void KEditor::ClipboardDocumentDCOPIface::clipboardChanged()
{
  m_iface->clipboardChanged();
}


void KEditor::ClipboardDocumentDCOPIface::forwardCopyAvailable(KEditor::Document *doc, bool available)
{
  QByteArray data;
  QDataStream arg(data, IO_WriteOnly);

  arg << doc->dcopInterface() << available;
  
  emitDCOPSignal("copyAvailable(DCOPRef,bool)", data);
}
#include "KEditorClipboardIface.moc"
