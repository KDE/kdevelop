#ifndef __DOCUMENTS_IFACE_H__
#define __DOCUMENTS_IFACE_H__


#include <qlist.h>


#include <kaction.h>


#include "interface.h"
#include "editor.h"


namespace KEditor {


class DocumentsEditorIface : public KEditor::EditorInterface
{
  Q_OBJECT

public:

  DocumentsEditorIface(Editor *parent);

  virtual QList<Document> documents() const = 0;

  virtual bool saveAllDocuments();


private slots:

  void slotSaveAll();

  void documentsChanged();
 
  void slotBufferSelected();


private:

  KAction *_saveAllAction;

};


}


#endif
