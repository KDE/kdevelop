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
