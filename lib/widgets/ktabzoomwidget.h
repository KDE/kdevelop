#ifndef __K_TABZOOM_WIDGET_H__
#define __K_TABZOOM_WIDGET_H__


#include <qwidget.h>


#include "ktabzoomposition.h"


#include "ktabzoombar.h"


class KTabZoomWidgetPrivate;

class KTabZoomWidget : public QWidget
{
  Q_OBJECT

public:

  KTabZoomWidget(QWidget *parent=0, KTabZoomPosition::Position pos=KTabZoomPosition::Left, const char *name=0);
  ~KTabZoomWidget();

  void addTab(QWidget *widget, const QString &tab);

  void addContent(QWidget *widget);

  void raiseWidget(QWidget *widget);
  void lowerWidget(QWidget *widget);

  void lowerAllWidgets();


private slots:

  void selected(int index);
  void unselected();

  void widgetDeleted();


protected:

  virtual void resizeEvent(QResizeEvent *ev);


private:

  void calculateGeometry();

  KTabZoomWidgetPrivate *d;

};


#endif
