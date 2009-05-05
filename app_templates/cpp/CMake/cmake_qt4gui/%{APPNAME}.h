#ifndef %{PROJECTDIRNAME}_%{APPNAME}_H
#define %{PROJECTDIRNAME}_%{APPNAME}_H

#include <QtGui/QMainWindow>

class %{APPNAME} : public QMainWindow
{
Q_OBJECT
public:
    %{APPNAME}();
    virtual ~%{APPNAME}();
};

#endif // %{PROJECTDIRNAME}_%{APPNAME}_H
