#include <kdebug.h>


#include "editor.h"


#include "KEditorEditIface.h"


static QCString generateName()
{
  static int id=0;
  QCString name = QString("KEditor::EditDocumentIface-%1").arg(++id).latin1();
  return name;
}


KEditor::EditDocumentDCOPIface::EditDocumentDCOPIface(KEditor::EditDocumentIface *iface)
  : QObject(iface), DCOPObject(generateName()), m_iface(iface)
{
}


KEditor::EditDocumentDCOPIface::~EditDocumentDCOPIface()
{
  kdDebug() << "Deleted KEditor::EditDocumentDCOPIface" << endl;
}


QString KEditor::EditDocumentDCOPIface::text()
{
  return m_iface->text();
}


void KEditor::EditDocumentDCOPIface::setText(const QString &text)
{
  m_iface->setText(text);
}


void KEditor::EditDocumentDCOPIface::append(const QString &text)
{
  m_iface->append(text);
}


bool KEditor::EditDocumentDCOPIface::insertLine(const QString &text, uint line)
{
  return m_iface->insertLine(text, line);
}


bool KEditor::EditDocumentDCOPIface::insertAt(const QString &text, uint line, uint col)
{
  return m_iface->insertAt(text, line, col);
}


bool KEditor::EditDocumentDCOPIface::removeLine(uint line)
{
  return m_iface->removeLine(line);
}


QString KEditor::EditDocumentDCOPIface::line(uint line) const
{
  return m_iface->line(line);
}


bool KEditor::EditDocumentDCOPIface::setLine(const QString &text, uint line)
{
  return m_iface->setLine(text, line);
}


#include "KEditorEditIface.moc"
