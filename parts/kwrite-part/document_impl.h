#ifndef __DOCUMENT_IMPL_H__
#define __DOCUMENT_IMPL_H__


#include <keditor/editor.h>


class KWriteDoc;
class KWrite;


class DocumentImpl : public KEditor::Document
{
  Q_OBJECT

public:

  DocumentImpl(KEditor::Editor *parent);
  virtual ~DocumentImpl();
  
  virtual bool load(QString filename);
  virtual bool save(QString filename);


private:

  KWriteDoc *m_document;
  KWrite *m_view;

};


#endif
