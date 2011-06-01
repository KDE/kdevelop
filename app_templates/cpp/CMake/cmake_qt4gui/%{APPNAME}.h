#ifndef %{APPNAME}_H
#define %{APPNAME}_H

#include <QtGui/QMainWindow>

class %{APPNAME} : public QMainWindow
{
Q_OBJECT
public:
    %{APPNAME}();
    virtual ~%{APPNAME}();
};

#endif // %{APPNAME}_H
