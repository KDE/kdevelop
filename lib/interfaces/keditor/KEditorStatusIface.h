#ifndef __KEDITOR_STATUS_IFACE_H__
#define __KEDITOR_STATUS_IFACE_H__


#include <qlist.h>


#include <dcopobject.h>
#include <dcopref.h>


#include "status_iface.h"


namespace KEditor
{


class StatusDocumentIface;


class StatusDocumentDCOPIface : public QObject, public DCOPObject
{
  Q_OBJECT
  K_DCOP

public:

  StatusDocumentDCOPIface(StatusDocumentIface *iface);
  ~StatusDocumentDCOPIface();

  
k_dcop:

  bool modified();
  QString status();


private slots:

  void forwardStatusChanged(Document *doc);
  void forwardMessage(Document *doc, const QString &text);


private:

  StatusDocumentIface *m_iface;

};


}


#endif
