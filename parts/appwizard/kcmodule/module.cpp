#include <qlayout.h>

#include <klocale.h>
#include <kglobal.h>

#include "$APPNAMELC$.h"

$APPNAME$::$APPNAME$(QWidget *parent, const char *name)
    : KCModule(parent, name)
{
    // place widgets here
    load();
};


$APPNAME$::~$APPNAME$()
{
}


void $APPNAME$::load()
{
    // insert your loading code here...
}


void $APPNAME$::defaults()
{
    // insert your default settings code here...
    emit changed(true);
}


void $APPNAME$::save()
{
    // insert your saving code here...
    emit changed(true);
}


int $APPNAME$::buttons()
{
    return KCModule::Default|KCModule::Apply|KCModule::Help;
}


void $APPNAME$::configChanged()
{
    // insert your saving code here...
    emit changed(true);
}


QString $APPNAME$::quickHelp() const
{
    return i18n("Helpful information about the $APPNAMELC$ module.");
}


extern "C"
{

    KCModule *create_$APPNAME$(QWidget *parent, const char *name)
    {
        return new $APPNAME$(parent, "$APPNAME$");
    }
}

#include "$APPNAMELC$.moc"
