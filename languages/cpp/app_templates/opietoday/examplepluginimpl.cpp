#include "%{APPNAMELC}.h"
#include "%{APPNAMELC}impl.h"

%{APPNAME}Impl::%{APPNAME}Impl() {
    examplePlugin  = new %{APPNAME}();
}

%{APPNAME}Impl::~%{APPNAME}Impl() {
    delete examplePlugin;
}


TodayPluginObject* %{APPNAME}Impl::guiPart() {
    return examplePlugin;
}

QRESULT %{APPNAME}Impl::queryInterface( const QUuid & uuid,  QUnknownInterface **iface ) {
    *iface = 0;
    if ( ( uuid == IID_QUnknown ) || ( uuid == IID_TodayPluginInterface ) )  {
        *iface = this, (*iface)->addRef();
    }else
	return QS_FALSE;

    return QS_OK;

}

Q_EXPORT_INTERFACE() {
    Q_CREATE_INSTANCE( %{APPNAME}Impl );
}
