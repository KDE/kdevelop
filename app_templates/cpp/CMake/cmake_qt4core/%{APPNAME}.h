#ifndef %{APPNAME}_H
#define %{APPNAME}_H

#include <QtCore/QObject>

class %{APPNAME} : public QObject
{
Q_OBJECT
public:
    %{APPNAME}();
    virtual ~%{APPNAME}();
private slots:
    void output();
};

#endif // %{APPNAME}_H
