#ifndef __K_TABZOOM_WIDGET_H__
#define __K_TABZOOM_WIDGET_H__


#include <qwidget.h>


class KConfig;


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

  void saveSettings(KConfig *config);
  void loadSettings(KConfig *config);
 
  bool isRaised() const;
  bool isDocked() const;
  bool hasFocus() const;
  bool isEmpty() const; //indicates whether widget has tabs

signals:
  // emitted when a tab gets added / deleted
  void tabsChanged();

public slots:

  void setDockMode(bool docked);
  void setFocus();

private slots:

  void selected(int index);
  void unselected();

  void widgetDeleted();

  void adjustStrut();


protected:

  virtual void resizeEvent(QResizeEvent *ev);


private:

  void calculateGeometry();

  KTabZoomWidgetPrivate *d;

};


#endif
