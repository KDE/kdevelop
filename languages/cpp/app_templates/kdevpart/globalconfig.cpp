%{CPP_TEMPLATE}
#include "%{APPNAMELC}globalconfig.h"

#include "%{APPNAMELC}part.h"

%{APPNAME}GlobalConfig::%{APPNAME}GlobalConfig(%{APPNAME}Part *part, QWidget *parent, const char *name)
    : %{APPNAME}GlobalConfigBase(parent, name), m_part(part)
{
}

void %{APPNAME}GlobalConfig::accept()
{
}

#include "%{APPNAMELC}globalconfig.moc"
