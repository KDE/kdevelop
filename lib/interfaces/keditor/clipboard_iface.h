#ifndef __CLIPBOARD_IFACE_H__
#define __CLIPBOARD_IFACE_H__


#include <kaction.h>


#include "interface.h"


namespace KEditor {


class ClipboardDocumentIface : public KEditor::DocumentInterface
{
  Q_OBJECT

public:

  ClipboardDocumentIface(Document *parent, Editor *editor);

  virtual bool copy() = 0;
  virtual bool cut() = 0;
  virtual bool paste() = 0;

  virtual bool copyAvailable() = 0;

  static ClipboardDocumentIface *interface(KEditor::Document *doc);


signals:

  void copyAvailable(KEditor::Document *doc, bool available);


public slots:
         
  void clipboardChanged();
 
 
private slots:
         
  void slotCopy();
  void slotCut();
  void slotPaste();


private:

  KAction *_cutAction, *_copyAction, *_pasteAction;
   
  bool hasClipboard();

};


}


#endif
