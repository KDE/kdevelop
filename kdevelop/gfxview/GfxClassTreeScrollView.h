#ifndef GFXCLASSTREESCROLLVIEW_H_INCLUDED
#define GFXCLASSTREESCROLLVIEW_H_INCLUDED


#include <qscrollview.h>
#include "GfxClassTree.h"
#include "../classparser/ClassStore.h"




class CGfxClassTreeScrollView : public QScrollView
{
  Q_OBJECT

 public:
  CGfxClassTree *m_classtree;
  
 public:
  /** Constructor */
  CGfxClassTreeScrollView(QWidget *aparentwidget);

  /** Destructor */
  ~CGfxClassTreeScrollView();

  /** implementation of resize event */
  virtual void resizeEvent(QResizeEvent *resevent);

};


#endif
