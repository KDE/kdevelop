#ifndef __DOCUMENT_IMPL_H__
#define __DOCUMENT_IMPL_H__


#include <ktexteditor/document.h>
#include <ktexteditor/editor.h>

class KWriteDoc;
class KWrite;

class DocumentImpl : public KTextEditor::Document
{
  Q_OBJECT

public:

  DocumentImpl(KTextEditor::Editor *parent, QWidget *parentWidget=0);
  virtual ~DocumentImpl();
  
  virtual bool saveFile();


protected:

  virtual bool openFile();


private:

  KWriteDoc *m_document;
  KWrite *m_view;

};


#endif
