
/* This template is based off of the KOffice example written by Torben Weis <weis@kde.org
   It was converted to a KDevelop template by Ian Reinhart Geiser <geiseri@yahoo.com>
*/

#include <koApplication.h>
#include <koDocument.h>
#include <koMainWindow.h>
#include <kcmdlineargs.h>
#include <klocale.h>
#include <dcopclient.h>
#include "%{APPNAMELC}_aboutdata.h"


static const KCmdLineOptions options[]=
{
	{"+[file]", I18N_NOOP("File to open"),0},
	KCmdLineLastOption
};

int main( int argc, char **argv )
{
    KCmdLineArgs::init( argc, argv, new%{APPNAME}AboutData() );
    KCmdLineArgs::addCmdLineOptions( options );
    KoApplication app;

    app.dcopClient()->attach();
    app.dcopClient()->registerAs( "%{APPNAMELC}" );

    if (!app.start()) // parses command line args, create initial docs and shells
	return 1;
    return app.exec();
}
