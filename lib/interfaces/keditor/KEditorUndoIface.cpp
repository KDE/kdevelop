#include <kdebug.h>


#include "editor.h"


#include "KEditorUndoIface.h"


static QCString generateName()
{
  static int id=0;
  QCString name = QString("KEditor::UndoDocumentIface-%1").arg(++id).latin1();
  return name;
}


KEditor::UndoDocumentDCOPIface::UndoDocumentDCOPIface(KEditor::UndoDocumentIface *iface)
  : QObject(iface), DCOPObject(generateName()), m_iface(iface)
{
  connect(m_iface, SIGNAL(undoAvailable(KEditor::Document *, bool)), this, SLOT(forwardUndoAvailable(KEditor::Document *, bool)));
  connect(m_iface, SIGNAL(redoAvailable(KEditor::Document *, bool)), this, SLOT(forwardRedoAvailable(KEditor::Document *, bool)));
}


KEditor::UndoDocumentDCOPIface::~UndoDocumentDCOPIface()
{
  kdDebug() << "Deleted KEditor::UndoDocumentDCOPIface" << endl;
}


bool KEditor::UndoDocumentDCOPIface::undo()
{
  return m_iface->undo();
}


bool KEditor::UndoDocumentDCOPIface::redo()
{
  return m_iface->redo();
}


bool KEditor::UndoDocumentDCOPIface::undoAvailable()
{
  return m_iface->undoAvailable();
}


bool KEditor::UndoDocumentDCOPIface::redoAvailable()
{
  return m_iface->redoAvailable();
}


void KEditor::UndoDocumentDCOPIface::forwardUndoAvailable(KEditor::Document *doc, bool available)
{
  QByteArray data;
  QDataStream arg(data, IO_WriteOnly);

  arg << doc->dcopInterface() << available;
  
  emitDCOPSignal("undoAvailable(DCOPRef,bool)", data);
}


void KEditor::UndoDocumentDCOPIface::forwardRedoAvailable(KEditor::Document *doc, bool available)
{
  QByteArray data;
  QDataStream arg(data, IO_WriteOnly);

  arg << doc->dcopInterface() << available;

  emitDCOPSignal("redoAvailable(DCOPRef,bool)", data);
}
        

#include "KEditorUndoIface.moc"
