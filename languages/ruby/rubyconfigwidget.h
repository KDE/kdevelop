
#ifndef RUBYCONFIGWIDGET_H
#define RUBYCONFIGWIDGET_H

#include "rubyconfigwidgetbase.h"
#include <qdom.h>

class RubyConfigWidget : public RubyConfigWidgetBase
{
  Q_OBJECT

public:
  RubyConfigWidget(QDomDocument &projectDom, QWidget* parent = 0, const char* name = 0);
  ~RubyConfigWidget();
  /*$PUBLIC_FUNCTIONS$*/

public slots:
  /*$PUBLIC_SLOTS$*/
  void accept();

protected:
  /*$PROTECTED_FUNCTIONS$*/

protected slots:
  /*$PROTECTED_SLOTS$*/

private:
  QDomDocument &dom;

};

#endif

