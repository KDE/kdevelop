#ifndef __KEDITOR_CURSOR_IFACE_H__
#define __KEDITOR_CURSOR_IFACE_H__


#include <qlist.h>


#include <dcopobject.h>
#include <dcopref.h>


#include "cursor_iface.h"


namespace KEditor
{


class CursorDocumentIface;


class CursorDocumentDCOPIface : public QObject, public DCOPObject
{
  Q_OBJECT
  K_DCOP

public:

  CursorDocumentDCOPIface(CursorDocumentIface *iface);
  ~CursorDocumentDCOPIface();

  
k_dcop:

  bool setCursorPosition(int line, int row);
  QPoint getCursorPosition();

  int numberOfLines();
  int lengthOfLine(int line);


private slots:

  void forwardCursorPositionChanged(Document *doc, int line, int col);


private:

  CursorDocumentIface *m_iface;

};


}


#endif
