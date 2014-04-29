#ifndef %{APPNAMEID}_H
#define %{APPNAMEID}_H

#include <QtCore/QObject>

class %{APPNAMEID} : public QObject
{
    Q_OBJECT

public:
    %{APPNAMEID}();
    virtual ~%{APPNAMEID}();

private slots:
    void output();
};

#endif // %{APPNAMEID}_H
