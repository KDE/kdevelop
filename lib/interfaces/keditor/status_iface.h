#ifndef __STATUS_IFACE_H__
#define __STATUS_IFACE_H__


#include "interface.h"


namespace KEditor {
		

class StatusDocumentIface : public KEditor::DocumentInterface
{
  Q_OBJECT

public:

  StatusDocumentIface(Document *parent, Editor *editor);

  virtual bool modified() = 0;
  virtual QString status() = 0;


signals:

  void statusChanged();
  void message(const QString &text);

};


}


#endif
