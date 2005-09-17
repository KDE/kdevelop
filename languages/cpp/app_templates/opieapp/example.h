#ifndef %{APPNAMEUC}_H
#define %{APPNAMEUC}_H
#include "%{APPNAMELC}base.h"

class %{APPNAME} : public %{APPNAME}Base
{
    Q_OBJECT

public:
    static QString appName() {
        return QString::fromLatin1("%{APPNAMELC}" );
    }
    %{APPNAME}( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    ~%{APPNAME}();

private slots:
    void goodBye();
};

#endif // %{APPNAMEUC}_H
