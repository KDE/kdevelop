#ifndef __DOCUMENT_IMPL_H__
#define __DOCUMENT_IMPL_H__

#include <keditor/editor.h>

class DocumentImpl : public KEditor::Document
{
    Q_OBJECT
  public: 
    DocumentImpl(KEditor::Editor *parent,QWidget *parentWidget=0);

    virtual bool saveFile();

    bool create();

    QString filename() { return m_file; }

  protected:
    virtual bool openFile();
 
    virtual bool shouldBeSaved();

  private:
    QString mFilename;
    
    static int mCounter;
};


#endif
