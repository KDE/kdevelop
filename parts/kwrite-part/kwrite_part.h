#ifndef __EDITORTEST_PART_H__
#define __EDITORTEST_PART_H__


#include <qlist.h>


#include <kparts/part.h>


class QTabWidget;
class KURL;


#include "keditor/editor.h"
#include "document_impl.h"


class KWritePart : public KEditor::Editor
{
  Q_OBJECT

public:

  KWritePart(QWidget *parentWidget, const char *widgetName, QObject *parent, const char *name);
  virtual ~KWritePart();


  virtual KEditor::Document *getDocument(const QString &filename=QString::null);
  virtual void closeDocument(KEditor::Document *doc);
  virtual KEditor::Document *currentDocument();


  QTabWidget *tabWidget() { return _stack; };

  QList <DocumentImpl> _documents;


private slots:

  void fileNameChanged(QString name);
  void currentChanged(QWidget *widget);


private:

  QTabWidget *_stack;

};


#endif
