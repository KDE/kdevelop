#include <kinstance.h>
#include <kaction.h>
#include <kstdaction.h>
#include <kfiledialog.h>
#include <kparts/partmanager.h>


#include <qfile.h>
#include <qtextstream.h>
#include <qmultilineedit.h>
#include <qtabwidget.h>


#include "editortest_part.h"
#include "editortest_factory.h"


#include "keditor/clipboard_iface.h"
#include "documents_iface_impl.h"


using namespace KEditor;


EditorTestPart::EditorTestPart(QObject *parent, const char *name )
  : Editor(parent, name)
{
  setInstance( EditorTestPartFactory::instance() );

  // create the interfaces
  new DocumentsIfaceImpl(this);

  setXMLFile("editortest_part.rc", true);

  _documents.setAutoDelete(true);
}


EditorTestPart::~EditorTestPart()
{
}


void EditorTestPart::currentChanged(QWidget *widget)
{
/*
  QListIterator<DocumentImpl> it(_documents);
  for ( ; it.current(); ++it)
	if (it.current()->editor() == widget)
	  emit documentActivated(it.current());		
*/
}


Document *EditorTestPart::getDocument(const QString &filename)
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
    impl = new DocumentImpl(this);

    if (!filename.isEmpty())
	impl->load(filename);
	
    connect(impl, SIGNAL(fileNameChanged(QString)), this, SLOT(fileNameChanged(QString)));

    _documents.append(impl);

	created = true;
  }
 
  if (created)
	emit Editor::documentAdded();

  emit Editor::activatePart(impl);

  if (impl->widget())
    emit Editor::activateView(impl->widget());
  
  return impl;
}


Document *EditorTestPart::currentDocument()
{
  QListIterator<DocumentImpl> it(_documents);
  for ( ; it.current(); ++it)
    if (it.current() == it.current()->manager()->activePart())
      return it.current();

  return 0;
}


void EditorTestPart::fileNameChanged(QString)
{
 /*  QListIterator<DocumentImpl> it(_documents);
   for ( ; it.current(); ++it)
     _stack->changeTab(it.current()->editor(), it.current()->shortName());
  */
}


#include "editortest_part.moc"
