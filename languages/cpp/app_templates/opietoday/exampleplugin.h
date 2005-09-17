#ifndef %{APPNAME}_PLUGIN_H
#define %{APPNAME}_PLUGIN_H

#include <qwidget.h>
#include <qguardedptr.h>

#include <opie/todayplugininterface.h>
#include <opie/todayconfigwidget.h>

#include "%{APPNAMELC}widget.h"

// implementation of the today plugin interface
class %{APPNAME} : public TodayPluginObject {

public:
    %{APPNAME}();
    ~%{APPNAME}();

    QString pluginName()  const;
    double versionNumber() const;
    QString pixmapNameWidget() const;
    QWidget* widget(QWidget *);
    QString pixmapNameConfig() const;
    TodayConfigWidget* configWidget(QWidget *);
    QString appName() const;
    bool excludeFromRefresh() const;
    void refresh();
    void reinitialize();

 private:
   QGuardedPtr<%{APPNAME}Widget> m_widget;
};

#endif
