#ifndef __STATUS_IFACE_IMPL_H__
#define __STATUS_IFACE_IMPL_H__


#include "keditor/status_iface.h"


class KWrite;


class StatusIfaceImpl : public KEditor::StatusDocumentIface
{
  Q_OBJECT

public:

  StatusIfaceImpl(KWrite *edit, KEditor::Document *parent, KEditor::Editor *editor);

  virtual bool modified();
  virtual QString status();


private slots:

  void slotStatusChanged();
  void slotMessage(const QString &text);


private:

  KWrite *m_edit;

};


#endif
