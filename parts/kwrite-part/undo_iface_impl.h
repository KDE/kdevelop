#ifndef __UNDO_IFACE_IMPL_H__
#define __UNDO_IFACE_IMPL_H__


#include "keditor/undo_iface.h"


class KWrite;


class UndoIfaceImpl : public KEditor::UndoDocumentIface
{
  Q_OBJECT

public:

  UndoIfaceImpl(KWrite *edit, KEditor::Document *parent, KEditor::Editor *editor);

  virtual bool undo();
  virtual bool redo();

  virtual bool undoAvailable();
  virtual bool redoAvailable();


private slots:

  void slotUndoStatus();


private:

  KWrite *m_edit;

  bool _undo, _redo;

};


#endif
