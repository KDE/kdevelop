#include <qmultilineedit.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qtabwidget.h>


#include "document_impl.h"
#include "editortest_part.h"
#include "clipboard_iface_impl.h"
#include "undo_iface_impl.h"
#include "cursor_iface_impl.h"


using namespace KEditor;


DocumentImpl::DocumentImpl(Editor *parent, QMultiLineEdit *widget)
  : Document(parent), _widget(widget)
{
  // create interfaces
  new ClipboardIfaceImpl(widget, this, parent);
  new UndoIfaceImpl(widget, this, parent);
  new CursorIfaceImpl(widget, this, parent);

  widget->setFocus();
}


bool DocumentImpl::load(QString filename)
{
  QFile f(filename);
  if (!f.open(IO_ReadOnly))
	return false;

  editor()->clear();
  
  QTextStream ts(&f);
  editor()->setText(ts.read());

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
  ts << editor()->text();

  f.close();

  rename(filename);

  return true;
}


#include "document_impl.moc"
