#ifndef __KWRITE_PART_H__
#define __KWRITE_PART_H__


#include <qlist.h>


#include <kparts/part.h>


class KURL;


#include "ktexteditor/editor.h"
#include "document_impl.h"


class KWritePart : public KTextEditor::Editor
{
  Q_OBJECT

public:

  KWritePart(QObject *parent, const char *name);
  virtual ~KWritePart();

  virtual KTextEditor::Document *document(const KURL &url);
  virtual KTextEditor::Document *createDocument(QWidget *parentWidget=0, const KURL &url="");


private slots:

  void documentDestroyed();


private:

  QList<DocumentImpl> _documents;
  
};


#endif
