#include <kinstance.h>
#include <kaction.h>
#include <kstdaction.h>
#include <kfiledialog.h>


#include <qfile.h>
#include <qtextstream.h>
#include <qmultilineedit.h>
#include <qtabwidget.h>


#include "editortest_part.h"
#include "editortest_factory.h"


#include "keditor/cursor_iface.h"
//#include "undo_iface_impl.h"
//#include "edit_iface_impl.h"
#include "keditor/clipboard_iface.h"
#include "keditor/undo_iface.h"


using namespace KEditor;


EditorTestPart::EditorTestPart(QWidget *parentWidget, const char *widgetName, QObject *parent, const char *name )
  : Editor(parentWidget, widgetName, parent, name)
{
  setInstance( EditorTestPartFactory::instance() );

  _stack = new QTabWidget(parentWidget);
  connect(_stack, SIGNAL(currentChanged(QWidget*)), this, SLOT(currentChanged(QWidget*)));
  
  // create the interfaces
  new CursorEditorIface(this);
  new UndoEditorIface(this);
  new ClipboardEditorIface(this);
  //  new EditIfaceImpl(m_widget, this);

  setWidget(_stack);

  setXMLFile("editortest_part.rc");

  _documents.setAutoDelete(true);

  _stack->setFocusPolicy(QWidget::ClickFocus);
}


EditorTestPart::~EditorTestPart()
{
}


void EditorTestPart::currentChanged(QWidget *widget)
{
  QListIterator<DocumentImpl> it(_documents);
  for ( ; it.current(); ++it)
	if (it.current()->editor() == widget)
	  emit documentActivated(it.current());		
}


Document *EditorTestPart::getDocument(const QString &filename)
{
  QMultiLineEdit *edit = 0;
  DocumentImpl *impl = 0;

  // look for an existing document with that name
  if (!filename.isEmpty())
  {
    QListIterator<DocumentImpl> it(_documents);
    for ( ; it.current(); ++it)
      if (it.current()->fileName() == filename)
 	{
	  impl = it.current();
	  break;
	}
  }

  // if there was none, create a new one
  if (!impl)
  {
    edit = new QMultiLineEdit(_stack);
    impl = new DocumentImpl(this, edit);

    if (!filename.isEmpty())
	impl->load(filename);
	
    connect(impl, SIGNAL(fileNameChanged(QString)), this, SLOT(fileNameChanged(QString)));

    _documents.append(impl);
    _stack->addTab(edit, impl->shortName());
  }
 
  // show the document 
  edit = impl->editor();
  _stack->showPage(edit);

  return impl;
}


void EditorTestPart::closeDocument(Document *doc)
{
  QListIterator<DocumentImpl> it(_documents);
  for ( ; it.current(); ++it)
	if (it.current() == doc)
	  {
		_stack->removePage(it.current()->editor());
	    delete it.current()->editor();
		_documents.remove(it.current());

		emit Editor::documentRemoved();
	  }
}


Document *EditorTestPart::currentDocument()
{
  QWidget *w = _stack->currentPage();
  if (!w)
    return 0;

  QListIterator<DocumentImpl> it(_documents);
  for ( ; it.current(); ++it)
    if (it.current()->editor() == w)
      return it.current();

  return 0;
}


void EditorTestPart::fileNameChanged(QString)
{
   QListIterator<DocumentImpl> it(_documents);
   for ( ; it.current(); ++it)
     _stack->changeTab(it.current()->editor(), it.current()->shortName());
}


#include "editortest_part.moc"
