#ifndef GFXCLASSTREEWINDOW_H_INCLUDED
#define GFXCLASSTREEWINDOW_H_INCLUDED


#include <qlist.h>
#include <qwidget.h>
#include <qpushbutton.h>
#include <qcombobox.h>
#include "GfxClassTreeScrollView.h"




class CGfxClassTreeWindow : public QWidget
{
  Q_OBJECT

 public:
  QPushButton *m_foldbtn;
  QPushButton *m_unfoldbtn;
  QComboBox *m_classcombo;
  CGfxClassTreeScrollView *m_treescrollview;

  CGfxClassTreeWindow(QWidget *aparent);
  ~CGfxClassTreeWindow();

  void InitializeTree(QList<CClassTreeNode> *forest);

  /** Implementation of resizeEvent() */
  virtual void resizeEvent(QResizeEvent *resevent);

  public slots:
   void foldClicked();
   void unfoldClicked();
   void itemSelected(int index);
};


#endif

