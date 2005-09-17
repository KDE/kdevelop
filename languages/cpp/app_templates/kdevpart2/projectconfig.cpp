%{CPP_TEMPLATE}
#include "%{APPNAMELC}projectconfig.h"

#include "%{APPNAMELC}part.h"

%{APPNAME}ProjectConfig::%{APPNAME}ProjectConfig(%{APPNAME}Part *part, QWidget *parent, const char *name)
    : %{APPNAME}ProjectConfigBase(parent, name), m_part(part)
{
}

void %{APPNAME}ProjectConfig::accept()
{
}

#include "%{APPNAMELC}projectconfig.moc"
