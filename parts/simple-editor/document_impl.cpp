#include <qmultilineedit.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qtabwidget.h>


#include <kparts/partmanager.h>


#include "document_impl.h"
#include "editortest_part.h"
#include "editortest_factory.h"
#include "clipboard_iface_impl.h"
#include "undo_iface_impl.h"
#include "cursor_iface_impl.h"


using namespace KEditor;


DocumentImpl::DocumentImpl(Editor *parent, QWidget *parentWidget)
  : Document(parent), _widget(0)
{
  setInstance(EditorTestPartFactory::instance());

  // create the editor
  _widget = new QMultiLineEdit(parentWidget);
  _widget->setFont(QFont("courier", 12));
  setWidget(_widget);  

  // create interfaces
  new ClipboardIfaceImpl(_widget, this, parent);
  new UndoIfaceImpl(_widget, this, parent);
  new CursorIfaceImpl(_widget, this, parent);

  setXMLFile("editortest_part.rc", true);
}


bool DocumentImpl::openFile()
{
  QFile f(m_file);
  if (!f.open(IO_ReadOnly))
	return false;

  _widget->clear();
  
  QTextStream ts(&f);
  _widget->setText(ts.read());

  f.close();

  return true;
}


bool DocumentImpl::saveFile()
{
  QFile f(m_file);
  if (!f.open(IO_WriteOnly))
	return false;

  QTextStream ts(&f);
  ts << _widget->text();

  f.close();

  return true;
}


#include "document_impl.moc"
