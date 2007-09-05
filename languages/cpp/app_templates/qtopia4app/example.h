
#ifndef %{APPNAMEUC}_H
#define %{APPNAMEUC}_H
#include "ui_%{APPNAMELC}base.h"

class %{APPNAME}Base : public QWidget, public Ui_%{APPNAME}Base
{
public:
    %{APPNAME}Base( QWidget *parent = 0, Qt::WFlags f = 0 );
    virtual ~%{APPNAME}Base();
};

class %{APPNAME} : public %{APPNAME}Base
{ 
    Q_OBJECT
public:
    %{APPNAME}( QWidget *parent = 0, Qt::WFlags f = 0 );
    virtual ~%{APPNAME}();

private slots:
    void goodBye();
};

#endif // %{APPNAMEUC}_H
