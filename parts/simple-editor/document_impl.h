#ifndef __DOCUMENT_IMPL_H__
#define __DOCUMENT_IMPL_H__


class QMultiLineEdit;


#include <keditor/editor.h>


class DocumentImpl : public KEditor::Document
{
  Q_OBJECT

public:

  DocumentImpl(KEditor::Editor *parent, QWidget *parentWidget=0);

  virtual bool saveFile();


protected:

  virtual bool openFile();


private:

  QMultiLineEdit *_widget;

};


#endif
