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


Editor::Editor(QObject *parent, const char *name)
  : QObject(parent, name), KXMLGUIClient((KXMLGUIClient*)parent)
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


#include "editor.moc"
