#ifndef __STATUS_IFACE_H__
#define __STATUS_IFACE_H__


#include "interface.h"


namespace KEditor {


class StatusDocumentDCOPIface;


class StatusDocumentIface : public KEditor::DocumentInterface
{
  Q_OBJECT

public:

  StatusDocumentIface(Document *parent, Editor *editor);

  virtual bool modified() = 0;
  virtual QString status() = 0;

  DCOPRef dcopInterface() const;

  
  static StatusDocumentIface *interface(Document *doc);


signals:

  void statusChanged(KEditor::Document *doc);
  void message(KEditor::Document *doc, const QString &text);


private:

  StatusDocumentDCOPIface *m_dcopIface;

};


}


#endif
