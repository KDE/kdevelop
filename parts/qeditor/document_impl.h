#ifndef __DOCUMENT_IMPL_H__
#define __DOCUMENT_IMPL_H__


class QEditor;


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

  QEditor *_widget;

};


#endif
