%{CPP_TEMPLATE}

#include <qlayout.h>

#include <klocale.h>
#include <kglobal.h>
#include <kparts/genericfactory.h>

#include "%{APPNAMELC}.h"

typedef KGenericFactory<%{APPNAME}, QWidget> %{APPNAME}Factory;
K_EXPORT_COMPONENT_FACTORY( kcm_%{APPNAME}, %{APPNAME}Factory("kcm%{APPNAMELC}"))

%{APPNAME}::%{APPNAME}(QWidget *parent, const char *name, const QStringList&)
    : KCModule(parent, name), myAboutData(0)
{
    // place widgets here
    load();
};


%{APPNAME}::~%{APPNAME}()
{
}


void %{APPNAME}::load()
{
    // insert your loading code here...
}


void %{APPNAME}::defaults()
{
    // insert your default settings code here...
    emit changed(true);
}


void %{APPNAME}::save()
{
    // insert your saving code here...
    emit changed(true);
}


int %{APPNAME}::buttons()
{
    return KCModule::Default|KCModule::Apply|KCModule::Help;
}


void %{APPNAME}::configChanged()
{
    // insert your saving code here...
    emit changed(true);
}


QString %{APPNAME}::quickHelp() const
{
    return i18n("Helpful information about the %{APPNAMELC} module.");
}


#include "%{APPNAMELC}.moc"
