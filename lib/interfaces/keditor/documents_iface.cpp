#include <kstdaction.h>
#include <kaction.h>
#include <kdebug.h>
#include <klocale.h>
#include <kiconloader.h>


#include <qapplication.h>


#include "editor.h"


#include "documents_iface.h"


using namespace KEditor;


DocumentsEditorIface::DocumentsEditorIface(Editor *parent)
  : EditorInterface(parent)
{
  _saveAllAction = new KAction(i18n("&Save All"), 0, this, SLOT(slotSaveAll()), actionCollection(), "file_save_all");

  connect(parent, SIGNAL(documentAdded()), this, SLOT(documentsChanged()));
  connect(parent, SIGNAL(documentRemoved()), this, SLOT(documentsChanged()));

  _saveAllAction->setEnabled(false);
}


bool DocumentsEditorIface::saveAllDocuments()
{
  // TODO: We should handle the case that we have multiple
  // Unnamed documents gracefully, perhaps by showing a list etc.
	
  bool ok = true;
 
  QList<Document> list = documents();
  QListIterator<Document> it(list);
  for ( ; it.current(); ++it)
  {
    kdDebug() << "SAVE : " << it.current()->shortName() << endl;
	if (!it.current()->save())
	  ok = false;
  }
  
  return ok;
}


void DocumentsEditorIface::documentsChanged()
{
  // Enable/Disable Save All action
  _saveAllAction->setEnabled(editor()->currentDocument());

  // Update the buffer menu
  editor()->unplugActionList("buffer_list");
  
  QList<KAction> bufferActions;
  QList<Document> list = documents();
  QListIterator<Document> it(list);
  for ( ; it.current(); ++it)
  {
	KAction *action = new KAction(it.current()->fileName(), 0, 0, it.current()->fileName().latin1());
	connect(action, SIGNAL(activated()), this, SLOT(slotBufferSelected()));
	bufferActions.append(action);
  }
  
  editor()->plugActionList("buffer_list", bufferActions);
}


void DocumentsEditorIface::slotBufferSelected()
{
  QString fileName = sender()->name();
  (void) editor()->getDocument(fileName);
}


void DocumentsEditorIface::slotSaveAll()
{
  (void) saveAllDocuments();
}
