#ifndef __KWRITE_PART_H__
#define __KWRITE_PART_H__


#include <qlist.h>


#include <kparts/part.h>


class KURL;


#include "keditor/editor.h"
#include "document_impl.h"


class KWritePart : public KEditor::Editor
{
  Q_OBJECT

public:

  KWritePart(QObject *parent, const char *name);
  virtual ~KWritePart();

  virtual KEditor::Document *document(const KURL &url);
  virtual KEditor::Document *createDocument(QWidget *parentWidget=0, const KURL &url="");
  virtual KEditor::Document *currentDocument();


private slots:

  void documentDestroyed();


private:

  QList<DocumentImpl> _documents;
  
};


#endif
