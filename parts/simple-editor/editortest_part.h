#ifndef __EDITORTEST_PART_H__
#define __EDITORTEST_PART_H__


#include <qlist.h>


#include <kparts/part.h>


class KURL;


#include "keditor/editor.h"
#include "document_impl.h"


class EditorTestPart : public KEditor::Editor
{
  Q_OBJECT

public:

  EditorTestPart(QObject *parent, const char *name);
  virtual ~EditorTestPart();

  virtual KEditor::Document *document(const KURL &url);
  virtual KEditor::Document *createDocument(QWidget *parentWidget=0, const KURL &url="");

  
private slots:

  void documentDestroyed();


private:

  QList<DocumentImpl> _documents;

};


#endif
