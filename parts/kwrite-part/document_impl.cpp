#include <qmultilineedit.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qtabwidget.h>


#include "document_impl.h"
#include "clipboard_iface_impl.h"
#include "undo_iface_impl.h"
#include "cursor_iface_impl.h"

#include "kwrite/kwdoc.h"
#include "kwrite/kwview.h"


using namespace KEditor;


DocumentImpl::DocumentImpl(Editor *parent, QWidget *parentWidget)
  : Document(parent)
{
  m_document = new KWriteDoc(HlManager::self(), "");
  m_view = static_cast<KWrite*>(m_document->createView(parentWidget, ""));

  // create interfaces
  new ClipboardIfaceImpl(m_view, this, parent);
  new CursorIfaceImpl(m_view, this, parent);
  new UndoIfaceImpl(m_view, this, parent);
}


DocumentImpl::~DocumentImpl()
{
  delete m_document;
}


bool DocumentImpl::load(QString filename)
{
  QFile f(filename);
  if (!f.open(IO_ReadOnly))
	return false;

  QTextStream ts(&f);
  m_document->setText(ts.read());

  f.close();

  rename(filename);
  
  return true;
}


bool DocumentImpl::save(QString filename)
{
  QFile f(filename);
  if (!f.open(IO_WriteOnly))
	return false;

  QTextStream ts(&f);
  ts << m_document->text();

  f.close();

  rename(filename);

  return true;
}


KWrite *DocumentImpl::widget()
{
  return m_view;
}


#include "document_impl.moc"
