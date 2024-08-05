#ifndef %{APPNAMEUC}_H
#define %{APPNAMEUC}_H

#include <interfaces/iplugin.h>

class %{APPNAMEID} : public KDevelop::IPlugin
{
    Q_OBJECT

public:
    // KPluginFactory-based plugin wants constructor with this signature
    %{APPNAMEID}(QObject* parent, const KPluginMetaData& metaData, const QVariantList& args);
};

#endif // %{APPNAMEUC}_H
