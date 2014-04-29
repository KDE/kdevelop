#ifndef %{APPNAMEID}_H
#define %{APPNAMEID}_H

#include <QtCore/QObject>

class %{APPNAMEID} : public QObject
{
    Q_OBJECT

public:
    %{APPNAMEID}();
    virtual ~%{APPNAMEID}();
};

#endif // %{APPNAMEID}_H
