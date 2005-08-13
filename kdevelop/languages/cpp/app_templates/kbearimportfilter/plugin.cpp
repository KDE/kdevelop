%{CPP_TEMPLATE}

//////////////////////////////////////////////////////////////////////
// Qt specific include files
#include <qdom.h>
//////////////////////////////////////////////////////////////////////
// KDE specific include files
#include <klocale.h>
#include <kmessagebox.h>
#include <kglobal.h>
#include <kgenericfactory.h>
#include <kdebug.h>
//////////////////////////////////////////////////////////////////////
// Application specific include files
#include "%{APPNAMELC}importfilterplugin.h"
#include <kbear/kbeartags.h>
#include <kbear/misc.h>

using namespace KBear;


typedef KGenericFactory<%{APPNAME}ImportFilterPlugin> %{APPNAME}ImportFilterPluginFactory;
K_EXPORT_COMPONENT_FACTORY( libkbear%{APPNAMELC}importfilter, %{APPNAME}ImportFilterPluginFactory( "kbear%{APPNAMELC}importfilter" ) );

%{APPNAME}ImportFilterPlugin::%{APPNAME}ImportFilterPlugin(QObject *parent, const char *name, const QStringList& )
    : SiteImportFilterPluginIface( parent, name ), m_hasError( false )
{
    KGlobal::locale()->insertCatalogue("kbear");
    m_domDocument.setContent( QString("<%1 %2=\"%3\"/>").arg( TAG_GROUP ).arg( ATT_LABEL ).arg(i18n("%{APPNAME} import")) );
}

%{APPNAME}ImportFilterPlugin::~%{APPNAME}ImportFilterPlugin()
{
}

QString %{APPNAME}ImportFilterPlugin::getDomDocument()
{
    return m_domDocument.toString();
}

void %{APPNAME}ImportFilterPlugin::import( const QString& fileName )
{
    // This method will be automatically called to start the import
    // If supplied fileName will contain the absolute path to the file to import
    if( fileName.isEmpty() || fileName.isNull() ) {
        KMessageBox::sorry( 0, i18n("You have to select a file to import."), i18n("No File") );
        m_hasError = true;
        emit progress( 100 );
        return;
    }

    // Here you should add your import code
    // you should also amit the progress signal to display the progress


    // Important !!!!!!
    // Never emit progress( 100 ) until you're totally finished since this will unload the plugin
    emit progress( 100 );
}

#include "%{APPNAMELC}importfilterplugin.moc"

