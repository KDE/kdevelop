#ifndef __QEDITOR_PART_H__
#define __QEDITOR_PART_H__


#include <qlist.h>


#include <kparts/part.h>


class KURL;


#include "keditor/editor.h"
#include "document_impl.h"


class QEditorPart : public KEditor::Editor
{
  Q_OBJECT

public:

  QEditorPart(QObject *parent, const char *name);
  virtual ~QEditorPart();

  virtual KEditor::Document *document(const KURL &url);
  virtual KEditor::Document *createDocument(QWidget *parentWidget=0, const KURL &url="");


private slots:

  void documentDestroyed();


private:

  QList<DocumentImpl> _documents;

};


#endif
