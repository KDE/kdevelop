#ifndef __KEDITOR_IFACE_H__
#define __KEDITOR_IFACE_H__


#include <qlist.h>


#include <dcopobject.h>
#include <dcopref.h>


#include "editor.h"


namespace KEditor
{


class DocumentDCOPIface : public QObject, public DCOPObject
{
  Q_OBJECT
  K_DCOP

public:

  DocumentDCOPIface(Document *document);
  ~DocumentDCOPIface();

  
k_dcop:

  DCOPRef queryInterface(const QString &iface);

  bool save();
  bool load(const KURL &url);
  QString url();


private:

  Document *m_document;

};


class EditorDCOPIface : public QObject, public DCOPObject
{
  Q_OBJECT
  K_DCOP

public:

  EditorDCOPIface(Editor *editor);
  ~EditorDCOPIface();

  
k_dcop:

  DCOPRef document(const KURL &url);
  DCOPRef currentDocument();
  
 
private:

  Editor *m_editor;
  
};


};


#endif
