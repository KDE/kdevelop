#ifndef %{APPNAME}_PLUGIN_IMPL_H
#define %{APPNAME}_PLUGIN_IMPL_H

#include <opie/todayplugininterface.h>

class %{APPNAME};

class %{APPNAME}Impl : public TodayPluginInterface{

public:
    %{APPNAME}Impl();
    virtual ~%{APPNAME}Impl();

    QRESULT queryInterface( const QUuid &,  QUnknownInterface** );
    Q_REFCOUNT

    virtual TodayPluginObject *guiPart();

private:
    %{APPNAME} *examplePlugin;
};

#endif
