#include "editor.h"


#include "interface.h"


using namespace KEditor;


EditorInterface::EditorInterface(Editor *parent)
  : QObject(parent), _editor(parent)
{
}



KActionCollection *EditorInterface::actionCollection()
{
  return _editor->actionCollection();
}


Editor *EditorInterface::editor()
{
  return _editor;
}



DocumentInterface::DocumentInterface(Document *parent, Editor *editor)
  : QObject(parent), _editor(editor), _document(parent)
{
}





#include "interface.moc"
