%{CPP_TEMPLATE}

#include <kurl.h>
#include <kapplication.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <klocale.h>

#include "testkmdi.h"

static const char description[] =
I18N_NOOP("A simple kmdi app");

static const char version[] = "0.0.1";

static KCmdLineOptions options[] =
{
	{ "+[URL]", I18N_NOOP( "Document to open." ), 0 },
	KCmdLineLastOption
};

int main(int argc, char **argv)
{
	KAboutData about("%{APPNAMELC}", I18N_NOOP("%{APPNAME}"), version, description,
			KAboutData::License_%{LICENSE}, "(C) %{YEAR} %{AUTHOR}", 0, 0, "%{EMAIL}" );
	about.addAuthor( "%{AUTHOR}", 0, "%{EMAIL}" );

	// warning, utf-8 char
	//about.addCredit( "bksys authors", "Thomas Nagy, Julien Antille" );

	KCmdLineArgs::init(argc, argv, &about);
	KCmdLineArgs::addCmdLineOptions(options);

	KApplication app;

	// see if we are starting with session management
	/*if (app.isRestored())
	  {
	  RESTORE((%{APPNAME});
	  } else */
	{
		// no session.. just start up normally
		KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

		testkmdi *widget = new testkmdi( KMdi::IDEAlMode );

		if (args->count() != 0)
		{
			for (int i=0; i < args->count(); i++)
			{
				widget->openURL( args->url( i ) );
			}
		}
		widget->show();
		args->clear();
	}

	return app.exec();
}

