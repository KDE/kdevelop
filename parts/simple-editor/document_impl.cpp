#include <qmultilineedit.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qtabwidget.h>


#include <kparts/partmanager.h>


#include "document_impl.h"
#include "editortest_part.h"
#include "clipboard_iface_impl.h"
#include "undo_iface_impl.h"
#include "cursor_iface_impl.h"


using namespace KEditor;


DocumentImpl::DocumentImpl(Editor *parent)
  : Document(parent), _widget(0)
{
  // create the editor
  _widget = new QMultiLineEdit;
  setWidget(_widget);  

  // register with the view manager
  parent->addView(_widget);

  // create interfaces
  new ClipboardIfaceImpl(_widget, this, parent);
  new UndoIfaceImpl(_widget, this, parent);
  new CursorIfaceImpl(_widget, this, parent);

  setXMLFile("editortest_part.rc", true);

  // register with the part manager
  parent->addPart(this);

  _widget->setFocus();
}


bool DocumentImpl::load(QString filename)
{
  QFile f(filename);
  if (!f.open(IO_ReadOnly))
	return false;

  _widget->clear();
  
  QTextStream ts(&f);
  _widget->setText(ts.read());

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
  ts << _widget->text();

  f.close();

  rename(filename);

  return true;
}


#include "document_impl.moc"
