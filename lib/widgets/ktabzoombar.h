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

  int addTab(const QTab &tab, const QString& toolTip = QString::null);
  void removeTab(int index);

  void setActiveIndex(int index);
  void unsetButtons();

  void setDockMode(bool docked);

private slots:
  void clicked(int index);

signals:
  void selected(int index);
  void unselected();
  void docked();

private:
  KTabZoomBarPrivate *d;

};


#endif
