#ifndef __DOCUMENT_IMPL_H__
#define __DOCUMENT_IMPL_H__


class QMultiLineEdit;


#include <keditor/editor.h>


class DocumentImpl : public KEditor::Document
{
  Q_OBJECT

public:

  DocumentImpl(KEditor::Editor *parent, QMultiLineEdit *widget);

  virtual bool load(QString filename);
  virtual bool save(QString filename);

  QMultiLineEdit *editor() { return _widget; };


private:

  QMultiLineEdit *_widget;

};


#endif
