#ifndef __KEDITOR_EDIT_IFACE_H__
#define __KEDITOR_EDIT_IFACE_H__


#include <qlist.h>


#include <dcopobject.h>
#include <dcopref.h>


#include "edit_iface.h"


namespace KEditor
{


class EditDocumentIface;


class EditDocumentDCOPIface : public QObject, public DCOPObject
{
  Q_OBJECT
  K_DCOP

public:

  EditDocumentDCOPIface(EditDocumentIface *iface);
  ~EditDocumentDCOPIface();

  
k_dcop:

  QString text();
  void setText(const QString &text);

  void append(const QString &text);
  bool insertLine(const QString &text, uint line);
  bool insertAt(const QString &text, uint line, uint col);
         
  bool removeLine(uint line);
           
  QString line(uint line) const;
  bool setLine(const QString &text, uint line);


private:

  EditDocumentIface *m_iface;

};


}


#endif
