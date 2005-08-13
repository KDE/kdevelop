%{CPP_TEMPLATE}

//////////////////////////////////////////////////////////////////////
// Qt specific include files
#include <qvbox.h>
//////////////////////////////////////////////////////////////////////
// System specific include files
#include <klocale.h>
#include <kinstance.h>
#include <kiconloader.h>
#include <kgenericfactory.h>
#include <kstatusbar.h>
#include <kdialogbase.h>
#include <kwizard.h>
//////////////////////////////////////////////////////////////////////
// Application specific include files
#include <kbear/misc.h>
#include <kbear/kbearapi.h>
#include <kbear/kbearcore.h>
#include <kbear/kbearmainwiniface.h>

#include "kbear%{APPNAMELC}outputwidget.h"
#include "kbear%{APPNAMELC}configwidget.h"
#include "kbear%{APPNAMELC}plugin.h"


using namespace KBear;


typedef KGenericFactory<KBear%{APPNAME}Plugin> KBear%{APPNAME}PluginFactory;
K_EXPORT_COMPONENT_FACTORY( libkbear%{APPNAMELC}, KBear%{APPNAME}PluginFactory( "kbear%{APPNAMELC}" ) );

KBear%{APPNAME}Plugin::KBear%{APPNAME}Plugin(QObject *parent, const char*, const QStringList& )
    : KBearPlugin( parent, "KBear%{APPNAME}Plugin" )
{
    setInstance(KBear%{APPNAME}PluginFactory::instance());
    setXMLFile("kbear%{APPNAMELC}.rc");

    m_outputWidget = new KBear%{APPNAME}OutputWidget( 0L, "KBear%{APPNAME}OutputWidget" );
}

KBear%{APPNAME}Plugin::~KBear%{APPNAME}Plugin()
{
    // if you have an embedded outputview you need to remove it here
//    mainWindow()->removeOutputPluginView( m_widget );
    // You're also responsible to delete it
	delete m_outputWidget;
}

void KBear%{APPNAME}Plugin::slotInit()
{
    connect( actionCollection(), SIGNAL( actionStatusText(const QString &) ), mainWindow()->statusBar(), SLOT( message(const QString &) ) );
    connect( actionCollection(), SIGNAL( clearStatusText() ), mainWindow()->statusBar(), SLOT( clear() ) );
    // this method will be automatically called when the plugin gets loaded
    // if you need to do any initializatione stuff, this is the place to do it

    // if you want to embed a widget as an outputview, simply uncomment
    // the following line.
//    mainWindow()->embedOutputPluginView( m_outputWidget, "name that should appear", "enter a tooltip" ) );
}

QString KBear%{APPNAME}Plugin::unloadWarning() const
{
    return i18n("This unique feature will not work if this plugin is unloaded" );
}

void KBear%{APPNAME}Plugin::slotConfigWidget( KDialogBase* dlg )
{
    // If you don't need a config widget just remove this code
    QVBox* vbox = dlg->addVBoxPage(i18n("%{APPNAME} Settings"), QString::null,
        KGlobal::iconLoader()->loadIcon("kbear%{APPNAMELC}", KIcon::NoGroup, KIcon::SizeMedium) );
    KBear%{APPNAME}ConfigWidget* w = new KBear%{APPNAME}ConfigWidget( vbox, "%{APPNAME}SettingsWidget" );
    connect( dlg, SIGNAL( okClicked() ), w, SLOT( slotSaveSettings() ) );
}

void KBear%{APPNAME}Plugin::slotConfigWidget( KWizard* wiz )
{
    // If you don't need a config widget just remove this code
    KBear%{APPNAME}ConfigWidget* w = new KBear%{APPNAME}ConfigWidget( wiz, "%{APPNAME}SettingsWidget" );
    wiz->addPage( w, i18n("%{APPNAME} Settings") );
    // when embedded in wizard, saveSettings() will be called automatically if needed
}


#include "kbear%{APPNAMELC}plugin.moc"

