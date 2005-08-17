%{CPP_TEMPLATE}

#include <stdlib.h>
#include <qcheckbox.h>
#include <qcolor.h>
#include <kapplication.h>
#include <klocale.h>
#include <kpushbutton.h>
#include <kconfig.h>
#include <kglobal.h>
#include "%{APPNAMELC}.h"
#include "%{APPNAMELC}ui.h"

//! libkscreensaver interface
extern "C"
{
    const char *kss_applicationName = "%{APPNAMELC}.kss";
    const char *kss_description = I18N_NOOP( "%{APPNAME}" );
    const char *kss_version = "2.2.0";

    %{APPNAME} *kss_create( WId id )
    {
        KGlobal::locale()->insertCatalogue("%{APPNAMELC}");
        return new %{APPNAME}( id );
    }

    QDialog *kss_setup()
    {
        KGlobal::locale()->insertCatalogue("%{APPNAMELC}");
        return new %{APPNAME}Setup();
    }
}

//-----------------------------------------------------------------------------
//! dialog to setup screen saver parameters
%{APPNAME}Setup::%{APPNAME}Setup( QWidget *parent, const char *name )
        : %{APPNAME}UI( parent, name, TRUE )
{
    /// @todo
    //Connect your signals and slots here to configure the screen saver.
    connect( OkayPushButton, SIGNAL( released() ),
             SLOT( slotOkPressed() ) );
    connect( CancelPushButton, SIGNAL( released() ),
             SLOT( slotCancelPressed() ) );
}


//! read settings from config file
void %{APPNAME}Setup::readSettings()
{
    KConfig *config = KGlobal::config();
    config->setGroup( "Settings" );
    /// @todo
    // Add your config options here...
    CheckBox1->setChecked(config->readBoolEntry( "somesetting", false ));
}


//! Ok pressed - save settings and exit
void %{APPNAME}Setup::slotOkPressed()
{
    KConfig *config = KGlobal::config();
    config->setGroup( "Settings" );
    /// @todo
    // Add your config options here.
    config->writeEntry( "somesetting", CheckBox1->isChecked() );
    config->sync();

    accept();
}

void %{APPNAME}Setup::slotCancelPressed()
{
    reject();
}
//-----------------------------------------------------------------------------


%{APPNAME}::%{APPNAME}( WId id ) : KScreenSaver( id )
{
    readSettings();
    blank();
}

%{APPNAME}::~%{APPNAME}()
{}


//! read configuration settings from config file
void %{APPNAME}::readSettings()
{
    KConfig *config = KGlobal::config();
    config->setGroup( "Settings" );
    /// @todo
    // Add your config options here...
    bool somesetting = config->readBoolEntry( "somesetting", false );
}


void %{APPNAME}::blank()
{
    /// @todo
    //Add your code to render the screen.
    setBackgroundColor( QColor(Qt::black)  );
    //
    erase();
}
