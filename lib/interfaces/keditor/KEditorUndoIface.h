#ifndef __KEDITOR_UNDO_IFACE_H__
#define __KEDITOR_UNDO_IFACE_H__


#include <qlist.h>


#include <dcopobject.h>
#include <dcopref.h>


#include "undo_iface.h"


namespace KEditor
{


class UndoDocumentIface;


class UndoDocumentDCOPIface : public QObject, public DCOPObject
{
  Q_OBJECT
  K_DCOP

public:

  UndoDocumentDCOPIface(UndoDocumentIface *iface);
  ~UndoDocumentDCOPIface();

  
k_dcop:

  bool undo();
  bool redo();

  bool undoAvailable();
  bool redoAvailable();
  

private slots:

  void forwardUndoAvailable(KEditor::Document *doc, bool available);
  void forwardRedoAvailable(KEditor::Document *doc, bool available);
  

private:

  UndoDocumentIface *m_iface;

};


}


#endif
