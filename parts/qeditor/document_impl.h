#ifndef __DOCUMENT_IMPL_H__
#define __DOCUMENT_IMPL_H__


class QEditor;

#include <keditor/editor.h>
#include <qfont.h>

class DocumentImpl : public KEditor::Document
{
  Q_OBJECT

public:

  DocumentImpl(KEditor::Editor *parent, QWidget *parentWidget=0);

  virtual bool saveFile();


protected:

  virtual bool openFile();
  
protected slots:
  void slotFontSettings( );


private:

  QEditor *_widget;
  QFont   m_font;

};


#endif
