#ifndef __INTERFACE_H__
#define __INTERFACE_H__


#include <qobject.h>


class KActionCollection;


namespace KEditor {


class Editor;
class Document;


class EditorInterface : public QObject
{
  Q_OBJECT

public:

  EditorInterface(Editor *parent);

  KActionCollection *actionCollection();

  QWidget *widget();
  
  Editor *editor();
  

private:

  Editor *_editor;
  
};


class DocumentInterface : public QObject
{
  Q_OBJECT

public:

  DocumentInterface(Document *parent, Editor *editor);

  Document *document() { return _document; };


private:

  Editor *_editor;
  Document *_document;
  
};


}


#endif
