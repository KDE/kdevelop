#ifndef __K_TABZOOMBUTTON_H__
#define __K_TABZOOMBUTTON_H__


#include <qpushbutton.h>


#include "ktabzoomposition.h"


class KTabZoomButtonPrivate;

class KTabZoomButton : public QPushButton
{
  Q_OBJECT

public:

  KTabZoomButton(const QString &text, QWidget *parent=0, KTabZoomPosition::Position pos=KTabZoomPosition::Left, const char *name=0);
  ~KTabZoomButton();


protected:

  virtual void drawButtonLabel(QPainter *);
  virtual void drawButton(QPainter *);
  virtual QSize sizeHint() const;


private:

  KTabZoomButtonPrivate *d;

};


#endif
