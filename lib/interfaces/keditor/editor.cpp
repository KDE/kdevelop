#include <klocale.h>


#include <qwidget.h>
#include <qobjectlist.h>


#include <kaction.h>
#include <kstdaction.h>
#include <kfiledialog.h>
#include <kdebug.h>
#include <kmessagebox.h>


#include "editor.h"


using namespace KEditor;


Document::Document(Editor *parent)
  : KParts::ReadWritePart(0), _parent(parent)
{
  KStdAction::saveAs(this, SLOT(slotSaveAs()), actionCollection(), "file_save_as");
  KStdAction::save(this, SLOT(slotSave()), actionCollection(), "file_save");
}


DocumentInterface *Document::queryInterface(const QString &ifname)
{
  if (!children())
    return 0;
           
  QObjectListIt it(*children());
  for ( ; it.current(); ++it)
    if (it.current()->inherits("KEditor::DocumentInterface") && it.current()->inherits(ifname))
      return (DocumentInterface*)it.current();
                 
  return 0;
}


void Document::slotSaveAs()
{
  KURL url = KFileDialog::getSaveURL();
  if (url.isEmpty())
    return;

  saveAs(url);
}


void Document::slotSave()
{
  save();
}


void Document::resetModifiedTime()
{
  if (!url().isLocalFile())
    return;

  struct stat buf;
  ::stat(url().path(), &buf);
  _mtime = buf.st_mtime;  
}


bool Document::shouldBeSaved()
{
  // Note: I don't test if the file has been modified in
  // the editor. This is a matter of opinion, but I think
  // if the users says "Save!", the file should be saved.     
        
  // we don't know about remote files, so better save
  if (!url().isLocalFile())
    return true;

  // if the file has not been modified on disk, save as well
  struct stat buf;
  ::stat(url().path(), &buf);
  if (buf.st_mtime == _mtime)
    return true;

  // it the file has been modified on disk, ask the user
  return KMessageBox::Yes == KMessageBox::warningYesNo(0, 
        i18n("The file %1 was modified on disk.\n"
             "Save the file anyway?").arg(url().path()));
  
}


Editor::Editor(QObject *parent, const char *name)
  : QObject(parent, name), KXMLGUIClient((KXMLGUIClient*)parent), _currentDocument(0)
{
}


Editor::~Editor()
{
}


EditorInterface *Editor::queryInterface(const QString &ifname)
{
  if (!children())
    return 0;

  QObjectListIt it(*children());
  for ( ; it.current(); ++it)
    if (it.current()->inherits("KEditor::EditorInterface") && it.current()->inherits(ifname))
      return (EditorInterface*)it.current();

  return 0;  
}


Document *Editor::currentDocument()
{
  return _currentDocument;
}


void Editor::activePartChanged(KParts::Part *part)
{
  if (!part || !part->inherits("KEditor::Document"))
    _currentDocument = 0;
  else 
    _currentDocument = static_cast<Document*>(part);
  
  emit documentActivated(_currentDocument);
}
  

#include "editor.moc"
