%{H_TEMPLATE}
#ifndef %{APPNAMEUC}_PROJECT_CONFIG_H
#define %{APPNAMEUC}_PROJECT_CONFIG_H

#include "%{APPNAMELC}projectconfigbase.h"

class %{APPNAME}Part;

class %{APPNAME}ProjectConfig: public %{APPNAME}ProjectConfigBase
{
    Q_OBJECT
public:
    %{APPNAME}ProjectConfig(%{APPNAME}Part *part, QWidget *parent = 0, const char *name = 0);

public slots:
    void accept();

private:
    %{APPNAME}Part *m_part;
};

#endif
