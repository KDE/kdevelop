#ifndef __example1_h__
#define __example1_h__

#include <qtabwidget.h>
#include <ktexteditor.h>
#include "docmanager.h"
#include <qlist.h>

class ViewManager : public QTabWidget
{
  Q_OBJECT
public:
  ViewManager(QWidget *parent, DocManager *dm);
  ~ViewManager();

  void createView (KTextEditor::Document *doc);
  void closeView (KTextEditor::View *view);

private:
  QList<KTextEditor::View> views;
  DocManager *dm;

public slots:
  void slotDocumentNew();
  void slotDocumentOpen();
};

#endif
