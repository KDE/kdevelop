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
  : KParts::Part(0), _parent(parent), _fileName("")
{
  KStdAction::save(this, SLOT(slotSave()), actionCollection(), "file_save");
  KStdAction::saveAs(this, SLOT(slotSaveAs()), actionCollection(), "file_save_as");
  KStdAction::close(this, SLOT(slotClose()), actionCollection(), "file_close");
}


DocumentInterface *Document::getInterface(const QString &ifname)
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


void Document::rename(const QString &filename)
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


void Document::slotSave()
{
  save();
}


void Document::slotSaveAs()
{
  QString fname = KFileDialog::getSaveFileName();
  if (fname.isEmpty())
    return;

  save(fname);
}


void Document::slotClose()
{
  delete this;
}


Editor::Editor(QObject *parent, const char *name)
  : QObject(parent), KXMLGUIClient((KXMLGUIClient*)parent)
{
  _openAction = KStdAction::open(this, SLOT(slotLoadFile()), actionCollection(), "file_open");
  _newAction = KStdAction::openNew(this, SLOT(slotNewFile()), actionCollection(), "file_new");
		  
  connect(this, SIGNAL(documentAdded()), this, SLOT(documentCountChanged()));
  connect(this, SIGNAL(documentRemoved()), this, SLOT(documentCountChanged()));
}


Editor::~Editor()
{
}


void Editor::documentCountChanged()
{
  Document *doc = currentDocument();
}


EditorInterface *Editor::getInterface(const QString &ifname)
{
  if (!children())
	return 0;

  QObjectListIt it(*children());
  for ( ; it.current(); ++it)
	if (it.current()->inherits("KEditor::EditorInterface") && it.current()->inherits(ifname))
	  return (EditorInterface*)it.current();

  return 0;  
}


void Editor::slotLoadFile()
{
  QString fname = KFileDialog::getOpenFileName();
  if (!fname.isEmpty())
	(void) getDocument(fname);
}
 
 
void Editor::slotNewFile()
{
  (void) getDocument();
}


void Editor::addView(QWidget *view)
{
  emit viewCreated(view);
}


void Editor::addPart(KParts::Part *part)
{
  emit partCreated(part);
}


#include "editor.moc"
