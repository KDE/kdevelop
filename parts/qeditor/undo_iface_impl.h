#ifndef __UNDO_IFACE_IMPL_H__
#define __UNDO_IFACE_IMPL_H__


#include <qeditor.h>


#include "keditor/undo_iface.h"


class UndoIfaceImpl : public KEditor::UndoDocumentIface
{
  Q_OBJECT

public:

  UndoIfaceImpl(QEditor *edit, KEditor::Document *parent, KEditor::Editor *editor);

  virtual bool undo();
  virtual bool redo();

  virtual bool undoAvailable();
  virtual bool redoAvailable();


private slots:

  void slotUndoAvailable(bool available);
  void slotRedoAvailable(bool available);


private:

  QEditor *m_edit;

  bool _undo, _redo;

};


#endif
