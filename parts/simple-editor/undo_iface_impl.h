#ifndef __UNDO_IFACE_IMPL_H__
#define __UNDO_IFACE_IMPL_H__


#include <qmultilineedit.h>


#include "keditor/undo_iface.h"


class UndoIfaceImpl : public KEditor::UndoDocumentIface
{
  Q_OBJECT

public:

  UndoIfaceImpl(QMultiLineEdit *edit, KEditor::Document *parent, KEditor::Editor *editor);

  virtual bool undo();
  virtual bool redo();

  virtual bool undoAvailable();
  virtual bool redoAvailable();


private slots:

  void slotUndoAvailable(bool available);
  void slotRedoAvailable(bool available);


private:

  QMultiLineEdit *m_edit;

  bool _undo, _redo;

};


#endif
