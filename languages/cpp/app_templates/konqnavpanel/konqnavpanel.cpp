
#include <qlabel.h>
#include <kinstance.h>
#include <qstring.h>
#include <qwidget.h>

#include "%{APPNAMELC}.h"
#include "%{APPNAMELC}.moc"

%{APPNAME}::%{APPNAME}(KInstance *inst,QObject *parent,QWidget *widgetParent, QString &desktopName, const char* name):
                   KonqSidebarPlugin(inst,parent,widgetParent,desktopName,name)
{
	widget=new QLabel("Init Value",widgetParent);
}


%{APPNAME}::~%{APPNAME}()
{
}

void %{APPNAME}::handleURL(const KURL &url)
{
	widget->setText(QString("%{APPNAME}")+"::"+url.url());
}



extern "C"
{
    void* create_konqsidebar_%{APPNAME}(KInstance *instance,QObject *par,QWidget *widp,QString &desktopname,const char *name)
    {
        return new %{APPNAME}(instance,par,widp,desktopname,name);
    }
};
