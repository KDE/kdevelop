#ifndef __EDITORTEST_PART_H__
#define __EDITORTEST_PART_H__


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
  virtual KEditor::Document *createDocument(const KURL &url="");
  virtual KEditor::Document *currentDocument();

};


#endif
