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


  virtual KEditor::Document *getDocument(const QString &filename=QString::null);
  virtual KEditor::Document *currentDocument();

  QList<DocumentImpl> _documents;


private slots:

  void activePartChanged(KParts::Part *part);


private:
		
  KEditor::Document *_currentDocument;


};


#endif
