#include <kinstance.h>
#include <kaction.h>
#include <kstdaction.h>
#include <kfiledialog.h>


#include <qfile.h>
#include <qtextstream.h>
#include <qmultilineedit.h>
#include <qtabwidget.h>


#include "kwrite/kwview.h"


#include "kwrite_part.h"
#include "kwrite_factory.h"


#include "keditor/cursor_iface.h"
#include "keditor/clipboard_iface.h"
#include "keditor/undo_iface.h"
#include "documents_iface_impl.h"


using namespace KEditor;


KWritePart::KWritePart(QWidget *parentWidget, const char *widgetName, QObject *parent, const char *name )
  : Editor(parentWidget, widgetName, parent, name)
{
  setInstance( KWritePartFactory::instance() );

  _stack = new QTabWidget(parentWidget);
  connect(_stack, SIGNAL(currentChanged(QWidget*)), this, SLOT(currentChanged(QWidget*)));
  
  // create the interfaces
  new CursorEditorIface(this);
  new UndoEditorIface(this);
  new ClipboardEditorIface(this);
  new DocumentsIfaceImpl(this);


  setWidget(_stack);

  setXMLFile("kwriteeditor_part.rc");

  _documents.setAutoDelete(true);

  _stack->setFocusPolicy(QWidget::ClickFocus);
}


KWritePart::~KWritePart()
{
}


void KWritePart::currentChanged(QWidget *widget)
{
  QListIterator<DocumentImpl> it(_documents);
  for ( ; it.current(); ++it)
	if (it.current()->widget() == widget)
	  emit documentActivated(it.current());		
}


Document *KWritePart::getDocument(const QString &filename)
{
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
  bool created = false;
  if (!impl)
  {
    impl = new DocumentImpl(this, _stack);

    if (!filename.isEmpty())
	impl->load(filename);
	
    connect(impl, SIGNAL(fileNameChanged(QString)), this, SLOT(fileNameChanged(QString)));

    _documents.append(impl);
    _stack->addTab(impl->widget(), impl->shortName());

	created = true;
  }
 
  // show the document 
  _stack->showPage(impl->widget());

  if (created)
	emit Editor::documentAdded();

  return impl;
}


void KWritePart::closeDocument(Document *doc)
{
  QListIterator<DocumentImpl> it(_documents);
  for ( ; it.current(); ++it)
	if (it.current() == doc)
	  {
		_stack->removePage(it.current()->widget());
	    delete it.current()->widget();
		_documents.remove(it.current());

		emit Editor::documentRemoved();
	  }
}


Document *KWritePart::currentDocument()
{
  QWidget *w = _stack->currentPage();
  if (!w)
    return 0;

  QListIterator<DocumentImpl> it(_documents);
  for ( ; it.current(); ++it)
    if (it.current()->widget() == w)
      return it.current();

  return 0;
}


void KWritePart::fileNameChanged(QString)
{
   QListIterator<DocumentImpl> it(_documents);
   for ( ; it.current(); ++it)
     _stack->changeTab(it.current()->widget(), it.current()->shortName());
}


#include "kwrite_part.moc"
