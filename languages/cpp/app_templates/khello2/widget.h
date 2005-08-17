%{H_TEMPLATE}

#ifndef _%{APPNAMEUC}WIDGET_H_
#define _%{APPNAMEUC}WIDGET_H_

#include "%{APPNAMELC}widgetbase.h"

class %{APPNAME}Widget : public %{APPNAME}WidgetBase
{
    Q_OBJECT

public:
    %{APPNAME}Widget(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = 0 );
    ~%{APPNAME}Widget();
    /*$PUBLIC_FUNCTIONS$*/

public slots:
    /*$PUBLIC_SLOTS$*/
    virtual void button_clicked();

protected:
    /*$PROTECTED_FUNCTIONS$*/

protected slots:
    /*$PROTECTED_SLOTS$*/

};

#endif

