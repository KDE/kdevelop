#ifndef __INTERFACE_H__
#define __INTERFACE_H__


#include <qobject.h>
#include <dcopref.h>


class KActionCollection;


namespace KEditor {


class Editor;
class Document;


class EditorInterface : public QObject
{
  Q_OBJECT

public:

  EditorInterface(Editor *parent);

  KActionCollection *actionCollection() const;

  Editor *editor() const;
  
  virtual DCOPRef dcopInterface() const = 0;


private:

  Editor *_editor;
  
};


class DocumentInterface : public QObject
{
  Q_OBJECT

public:

  DocumentInterface(Document *parent, Editor *editor);

  Document *document() const { return _document; };

  virtual DCOPRef dcopInterface() const { return DCOPRef(); }; // TODO: make abstract!

  
private:

  Editor *_editor;
  Document *_document;
  
};


}


#endif
