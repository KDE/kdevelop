#include <kdebug.h>


#include "editor.h"


#include "KEditorCursorIface.h"


static QCString generateName()
{
  static int id=0;
  QCString name = QString("KEditor::CursorDocumentIface-%1").arg(++id).latin1();
  return name;
}


KEditor::CursorDocumentDCOPIface::CursorDocumentDCOPIface(KEditor::CursorDocumentIface *iface)
  : QObject(iface), DCOPObject(generateName()), m_iface(iface)
{
  connect(m_iface, SIGNAL(cursorPositionChanged(KEditor::Document *,int,int)), this, SLOT(forwardCursorPositionChanged(KEditor::Document *,int,int)));
}


KEditor::CursorDocumentDCOPIface::~CursorDocumentDCOPIface()
{
  kdDebug() << "Deleted KEditor::CursorDocumentDCOPIface" << endl;
}


bool KEditor::CursorDocumentDCOPIface::setCursorPosition(int line, int col)
{
  return m_iface->setCursorPosition(line, col);
}


QPoint KEditor::CursorDocumentDCOPIface::getCursorPosition()
{
  int line, col;
  m_iface->getCursorPosition(line, col);
  return QPoint(line,col);
}


int KEditor::CursorDocumentDCOPIface::numberOfLines()
{
  return m_iface->numberOfLines();
}


int KEditor::CursorDocumentDCOPIface::lengthOfLine(int line)
{
  return m_iface->lengthOfLine(line);
}


void KEditor::CursorDocumentDCOPIface::forwardCursorPositionChanged(KEditor::Document *doc, int line, int col)
{
  QByteArray data;
  QDataStream arg(data, IO_WriteOnly);

  arg << doc->dcopInterface() << line << col;
  
  emitDCOPSignal("cursorPositionChanged(DCOPRef,int,int)", data);
}
