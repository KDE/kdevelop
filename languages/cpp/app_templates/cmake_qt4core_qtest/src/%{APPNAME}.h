#ifndef %{PROJECTDIRNAME}_%{APPNAME}_H
#define %{PROJECTDIRNAME}_%{APPNAME}_H

#include <QtCore/QObject>

class %{APPNAME} : public QObject
{
Q_OBJECT
public:
    %{APPNAME}();
    virtual ~%{APPNAME}();
};

#endif // %{PROJECTDIRNAME}_%{APPNAME}_H
