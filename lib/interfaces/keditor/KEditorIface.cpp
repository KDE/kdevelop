#include <kdebug.h>


#include "editor.h"


#include "KEditorIface.h"


static QCString generateName()
{
  static int id = 0;
  QCString name = QString("KEditor::Document-%1").arg(++id).latin1();
  return name;
}


KEditor::DocumentDCOPIface::DocumentDCOPIface(KEditor::Document *document)
  : QObject(document), DCOPObject(generateName()), m_document(document)
{
}


KEditor::DocumentDCOPIface::~DocumentDCOPIface()
{
  kdDebug() << "Deleted KEditor::DocumentDCOPIface" << endl;
}


DCOPRef KEditor::DocumentDCOPIface::queryInterface(const QString &iface)
{
  KEditor::DocumentInterface *i = m_document->queryInterface(iface);
  if (!i)
    return DCOPRef();

  return i->dcopInterface();
}


bool KEditor::DocumentDCOPIface::save()
{
  bool result = m_document->save();
  if (result)
    m_document->setModified(false);

  return result;
}


bool KEditor::DocumentDCOPIface::load(const KURL &url)
{
  return m_document->openURL(url);
}


QString KEditor::DocumentDCOPIface::url()
{
  return m_document->url().url();
}


KEditor::EditorDCOPIface::EditorDCOPIface(KEditor::Editor *editor)
  : QObject(editor), DCOPObject("KEditor"), m_editor(editor)
{
}


KEditor::EditorDCOPIface::~EditorDCOPIface()
{
  kdDebug() << "Deleted KEditor::EditorDCOPIface" << endl;
}


DCOPRef KEditor::EditorDCOPIface::document(const KURL &url)
{
  KEditor::Document *doc = m_editor->document(url);
  if (!doc)
    return DCOPRef();

  return doc->dcopInterface();
}


DCOPRef KEditor::EditorDCOPIface::currentDocument()
{
  KEditor::Document *doc = m_editor->currentDocument();
  if (!doc)
    return DCOPRef();

  return doc->dcopInterface();
}



//#include "KEditorIface.moc.cpp"
