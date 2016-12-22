#ifndef %{APPNAMEUC}_H
#define %{APPNAMEUC}_H

#include <interfaces/iplugin.h>

class %{APPNAME} : public KDevelop::IPlugin
{
    Q_OBJECT

public:
    // KPluginFactory-based plugin wants constructor with this signature
    %{APPNAME}(QObject* parent, const QVariantList& args);
};

#endif // %{APPNAMEUC}_H
