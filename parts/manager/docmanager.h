#ifndef __example2_h__
#define __example2_h__

#include <qobject.h>

#include <ktexteditor.h>
#include <qlist.h>
#include <qstring.h>

class DocManager : public QObject
{
  Q_OBJECT
public:
  DocManager();
  ~DocManager();

  KTextEditor::Document *createDoc (QString type);
  bool deleteDoc (KTextEditor::Document *doc);

private:
  QList<KTextEditor::Document> docs;
};

#endif
