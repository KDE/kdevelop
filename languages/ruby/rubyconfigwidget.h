
#ifndef RUBYCONFIGWIDGET_H
#define RUBYCONFIGWIDGET_H

#include "rubyconfigwidgetbase.h"

class QDomDocument;

class RubyConfigWidget : public RubyConfigWidgetBase
{
  Q_OBJECT
public:
  RubyConfigWidget(QDomDocument &projectDom, QWidget* parent = 0, const char* name = 0);

public slots:
  void accept();

protected:

protected slots:

private:
  QDomDocument &dom;
};

#endif

