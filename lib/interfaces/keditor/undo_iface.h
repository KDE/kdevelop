#ifndef __UNDO_IFACE_H__
#define __UNDO_IFACE_H__


#include <kaction.h>


#include "interface.h"


namespace KEditor {


class UndoDocumentIface : public KEditor::DocumentInterface
{
  Q_OBJECT

public:

  UndoDocumentIface(Document *parent, Editor *editor);

  virtual bool undo() = 0;
  virtual bool redo() = 0;

  virtual bool undoAvailable() = 0;
  virtual bool redoAvailable() = 0;


signals:

  void undoAvailable(bool available);
  void redoAvailable(bool available);
  
};


class UndoEditorIface : public KEditor::EditorInterface
{
  Q_OBJECT

public:

  UndoEditorIface(Editor *parent);


public slots:

  void undoChanged();


private slots:
		
  void slotUndo();
  void slotRedo();


private:

  KAction *_undoAction, *_redoAction;

  UndoDocumentIface *documentIface();

};


}


#endif
