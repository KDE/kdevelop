#ifndef __K_TABZOOMBAR_H__
#define __K_TABZOOMBAR_H__


#include <qtabbar.h>


#include "ktabzoomposition.h"


class KTabZoomBarPrivate;

class KTabZoomBar : public QWidget
{
  Q_OBJECT

public:

  KTabZoomBar(QWidget *parent=0, KTabZoomPosition::Position pos=KTabZoomPosition::Left, const char *name=0);
  ~KTabZoomBar();

  int addTab(QTab *tab);
  void removeTab(int index);

  void setPressed(int index, bool pressed=true);
  
  void unsetButtons();


signals:

  void selected(int index);
  void unselected();


private slots:

  void toggled(int index);


private:

  KTabZoomBarPrivate *d;

};


#endif
