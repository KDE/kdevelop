#ifndef __KEDITOR_CLIPBOARD_IFACE_H__
#define __KEDITOR_CLIPBOARD_IFACE_H__


#include <qlist.h>


#include <dcopobject.h>
#include <dcopref.h>


#include "clipboard_iface.h"


namespace KEditor
{


class ClipboardDocumentIface;


class ClipboardDocumentDCOPIface : public QObject, public DCOPObject
{
  Q_OBJECT
  K_DCOP

public:

  ClipboardDocumentDCOPIface(ClipboardDocumentIface *iface);
  ~ClipboardDocumentDCOPIface();

  
k_dcop:

  void cut();
  void copy();
  void paste();

  void clipboardChanged();


private slots:

  void forwardCopyAvailable(Document *doc, bool available);


private:

  ClipboardDocumentIface *m_iface;

};


}


#endif
