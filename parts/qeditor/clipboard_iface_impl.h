#ifndef __CLIPBOARD_IFACE_IMPL_H__
#define __CLIPBOARD_IFACE_IMPL_H__


#include <qeditor.h>


#include "keditor/clipboard_iface.h"


class ClipboardIfaceImpl : public KEditor::ClipboardDocumentIface
{
  Q_OBJECT

public:

  ClipboardIfaceImpl(QEditor *edit, KEditor::Document *parent, KEditor::Editor *editor);

  virtual bool cut();
  virtual bool copy();
  virtual bool paste();

  virtual bool copyAvailable();


private slots:

  void slotCopyAvailable(bool avail);


private:

  QEditor *m_edit;

  bool _available;

};


#endif
