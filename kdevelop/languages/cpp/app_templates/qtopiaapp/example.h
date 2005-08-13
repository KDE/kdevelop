
#ifndef %{APPNAMEUC}_H
#define %{APPNAMEUC}_H
#include "%{APPNAMELC}base.h"

class %{APPNAME} : public %{APPNAME}Base
{ 
    Q_OBJECT

public:
    %{APPNAME}( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    ~%{APPNAME}();

private slots:
    void goodBye();
};

#endif // %{APPNAMEUC}_H
