%{H_TEMPLATE}
#ifndef %{APPNAMEUC}_GLOBAL_CONFIG_H
#define %{APPNAMEUC}_GLOBAL_CONFIG_H

#include "%{APPNAMELC}globalconfigbase.h"

class %{APPNAME}Part;

class %{APPNAME}GlobalConfig: public %{APPNAME}GlobalConfigBase
{
    Q_OBJECT
public:
    %{APPNAME}GlobalConfig(%{APPNAME}Part *part, QWidget *parent = 0, const char *name = 0);

public slots:
    void accept();

private:
    %{APPNAME}Part *m_part;
};

#endif
