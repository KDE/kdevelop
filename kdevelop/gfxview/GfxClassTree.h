#ifndef GFXCLASSTREE_H_INCLUDED
#define GFXCLASSTREE_H_INCLUDED

#include <qwidget.h>
#include "GfxClassBox.h"
#include "../classparser/ClassStore.h"


class CGfxClassTree : public QWidget
{
  Q_OBJECT

 public: // Constructor & Destructor

  /** Constructor */
  CGfxClassTree(QWidget *aparentwidget);

  /** Destructor */
  ~CGfxClassTree();

  /*===================================================*/

  QList<CGfxClassBox> m_boxlist;

 private: // Private methods
  /** Insert a box object in the class box tree */
  void InsertBox(CGfxClassBox *abox,
		 CGfxClassBox *aparentbox,
		 CGfxClassBox *asiblingbox);

 public: // Public methods

  /** Remove all class boxes in the tree */
  void RemoveAll();

  /** Get CGfxClassBox object on box id */
  CGfxClassBox *GetBoxId(int boxid);

  /** Insert a class tree */
  CGfxClassBox *InsertClassTree(CGfxClassBox *baseclassbox,
				CGfxClassBox *sibclassbox,
				CClassTreeNode *atreenode);

  /** Insert a class forest */
  void InsertClassForest(CGfxClassBox *baseclassbox,
			 QList<CClassTreeNode> *forest);
  
  /** Refresh the entire class tree view */
  void RefreshClassForest(QList<CClassTreeNode> *forest);

  /** Refresh tree widget size */
  void RefreshTreeSize();

  /** Fold or unfold all class boxes */
  void SetUnfoldAll(bool unfolded);

  /** Implementation of resizeEvent() */
  virtual void resizeEvent(QResizeEvent *);


  virtual void paintEvent(QPaintEvent *);

 signals:
  void PosRefresh(int); 

 public slots:
  void stateChange(CGfxClassBox *abox);
  void drawConnection(CGfxClassBox *abox);
};


#endif
