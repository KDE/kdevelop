%{CPP_TEMPLATE}

#include <config.h>
#include "kfile_%{APPNAMELC}.h"

#include <kgenericfactory.h>
//#include <kio/global.h>

//#include <qfileinfo.h>
//#include <qdir.h>

typedef KGenericFactory<%{APPNAME}Plugin> %{APPNAME}Factory;

K_EXPORT_COMPONENT_FACTORY(kfile_%{APPNAME}, %{APPNAME}Factory( "kfile_%{APPNAMELC}" ))

%{APPNAME}Plugin::%{APPNAME}Plugin(QObject *parent, const char *name,
                       const QStringList &args)
    : KFilePlugin(parent, name, args)
{
    //add the mimetype here - example:
    //KFileMimeTypeInfo* info = addMimeTypeInfo( "text/html" );
    KFileMimeTypeInfo* info = addMimeTypeInfo( "text/html" );

    // our new group
    KFileMimeTypeInfo::GroupInfo* group = 0L;
    group = addGroupInfo(info, "%{APPNAME}Info", i18n("%{APPNAME} Information"));

    KFileMimeTypeInfo::ItemInfo* item;

    // our new items in the group
    item = addItemInfo(group, "Items", i18n("Items"), QVariant::Int);
    item = addItemInfo(group, "Size", i18n("Size"), QVariant::Int);
    setUnit(item, KFileMimeTypeInfo::KiloBytes);

    // strings are possible, too:
    //addItemInfo(group, "Text", i18n("Document type"), QVariant::String);
}

bool %{APPNAME}Plugin::readInfo( KFileMetaInfo& info, uint /*what*/)
{
    KFileMetaInfoGroup group = appendGroup(info, "%{APPNAME}Info");

    // add your "calculations" here
    // if something goes wrong, "return false;"


    // and finally display it!
    appendItem(group, "Items", 100);
    appendItem(group, "Size", int(5000/1024));
    
    return true;
}

#include "kfile_%{APPNAMELC}.moc"

