
#include "%{APPNAMELC}.h"

%{APPNAME}::%{APPNAME}() {
    m_widget = 0l;
}

%{APPNAME}::~%{APPNAME}() {
    delete (%{APPNAME}Widget*)m_widget;
}

QString %{APPNAME}::pluginName() const {
    return QObject::tr( "%{APPNAME}" );
}

double %{APPNAME}::versionNumber() const {
    return 0.1;
}

// this sets the image that will be shown on the left side of the plugin
QString %{APPNAME}::pixmapNameWidget() const {
    return QString::fromLatin1("%{APPNAMELC}/%{APPNAMELC}");
}

QWidget* %{APPNAME}::widget( QWidget * wid ) {
    if(!m_widget) {
        m_widget = new %{APPNAME}Widget( wid,  "%{APPNAME}" );
    }
    return m_widget;
}


// that would be the icon of the config widget in todays config view
QString %{APPNAME}::pixmapNameConfig() const {
    return 0l;
}

// No config widget yet, look at the datebook plugin for an example of that
TodayConfigWidget* %{APPNAME}::configWidget( QWidget* /*parent*/ ) {
    return 0l;
}

// add the binary name of the app to launch here
QString %{APPNAME}::appName() const {
    return QString::null;
}

// if the plugin should be excluded form the refresh cycles that can be set in the today app
bool %{APPNAME}::excludeFromRefresh() const {
    return false;
}

void %{APPNAME}::refresh()  {
    if ( m_widget )  {
        m_widget->refresh();
    }
}

void %{APPNAME}::reinitialize()  {
}
