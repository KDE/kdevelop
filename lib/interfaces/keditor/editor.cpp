#include <klocale.h>


#include <qwidget.h>
#include <qobjectlist.h>


#include <kaction.h>
#include <kstdaction.h>
#include <kfiledialog.h>
#include <kdebug.h>


#include "editor.h"


using namespace KEditor;


Document::Document(Editor *parent)
  : QObject(parent), _fileName("")
{
}


DocumentInterface *Document::getInterface(QString ifname)
{
  if (!children())
	return 0;
		   
  QObjectListIt it(*children());
  for ( ; it.current(); ++it)
    if (it.current()->inherits("KEditor::DocumentInterface") && it.current()->inherits(ifname))
      return (DocumentInterface*)it.current();
  			   
  return 0;
}


bool Document::save()
{
  // if no filename has been set yet, ask for one
  if (_fileName.isEmpty())
  {
	QString fname = KFileDialog::getSaveFileName();
	if (fname.isEmpty())
	  return false;
	rename(fname);
  }

  return save(_fileName);
}


void Document::rename(QString filename)
{
  _fileName = filename;
  emit fileNameChanged(filename);
}


QString Document::fileName() const
{
  if (_fileName.isEmpty())
    return i18n("Unnamed");
  return _fileName;
}


QString Document::shortName() const
{
  // if no name has been set, us "Unnamed"
  if (_fileName.isEmpty())
	return i18n("Unnamed");

  // as default, we just use the basename of the file
  int pos = _fileName.findRev("/");
  if (pos >= 0)
	return _fileName.mid(pos+1);
  else
	return _fileName;
}



Editor::Editor(QWidget *parentWidget, const char *widgetName, QObject *parent, const char *name)
  : KParts::ReadOnlyPart(parent, name)
{
  _openAction = KStdAction::open(this, SLOT(slotLoadFile()), actionCollection(), "file_open");
  _saveAction = KStdAction::save(this, SLOT(slotSaveFile()), actionCollection(), "file_save");
  _saveAsAction = KStdAction::saveAs(this, SLOT(slotSaveFileAs()), actionCollection(), "file_save_as");
  _newAction = KStdAction::openNew(this, SLOT(slotNewFile()), actionCollection(), "file_new");
  _closeAction = KStdAction::close(this, SLOT(slotCloseFile()), actionCollection(), "file_close");
		  
  connect(this, SIGNAL(documentAdded()), this, SLOT(documentCountChanged()));
  connect(this, SIGNAL(documentRemoved()), this, SLOT(documentCountChanged()));

  _saveAction->setEnabled(false);
  _saveAsAction->setEnabled(false);
  _closeAction->setEnabled(false);
}


Editor::~Editor()
{
}


void Editor::documentCountChanged()
{
  Document *doc = currentDocument();
		   
  _saveAction->setEnabled(doc);
  _saveAsAction->setEnabled(doc);
  _closeAction->setEnabled(doc);
}


EditorInterface *Editor::getInterface(QString ifname)
{
  if (!children())
	return 0;

  QObjectListIt it(*children());
  for ( ; it.current(); ++it)
	if (it.current()->inherits("KEditor::EditorInterface") && it.current()->inherits(ifname))
	  return (EditorInterface*)it.current();

  return 0;  
}


bool Editor::openFile()
{
  return getDocument(m_file) != 0;
}


void Editor::slotLoadFile()
{
  QString fname = KFileDialog::getOpenFileName();
  if (!fname.isEmpty())
	(void) getDocument(fname);
}
 
 
void Editor::slotSaveFile()
{
  Document *doc = currentDocument();
  if (doc)
	(void) doc->save();
}
 
 
void Editor::slotSaveFileAs()
{
  Document *doc = currentDocument();
  if (!doc)
	return;

  QString fname = KFileDialog::getSaveFileName();
  if (fname.isEmpty())
	return;

  (void) doc->save(fname);
}
 
 
void Editor::slotNewFile()
{
  (void) getDocument();
}
 
 
void Editor::slotCloseFile()
{
  Document *doc = currentDocument();
  if (doc)
	closeDocument(doc);
}


#include "editor.moc"
