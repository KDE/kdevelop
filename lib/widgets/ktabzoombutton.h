#ifndef __K_TABZOOMBUTTON_H__
#define __K_TABZOOMBUTTON_H__


#include <qtoolbutton.h>


#include "ktabzoomposition.h"


class KTabZoomButtonPrivate;

class KTabZoomButton : public QToolButton
{
  Q_OBJECT

public:

  KTabZoomButton(const QString &text, QWidget *parent=0, KTabZoomPosition::Position pos=KTabZoomPosition::Left, const char *name=0);
  ~KTabZoomButton();


protected:

  virtual void drawButtonLabel(QPainter *);
  virtual QSize sizeHint() const;


private:

  KTabZoomButtonPrivate *d;

};


#endif
