%{CPP_TEMPLATE}

#include <kuniqueapplication.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kdebug.h>
#include <klocale.h>
#include <dcopclient.h>
#include "%{APPNAMELC}.h"

static const char description[] =
    I18N_NOOP("A KDE KPart Application");

static const char version[] = "0.1";

static KCmdLineOptions options[] =
{
    KCmdLineLastOption
};

int main (int argc, char *argv[])
{
	KLocale::setMainCatalogue("kdelibs");
	KAboutData aboutdata("%{APPNAMELC}", I18N_NOOP("KDE"),
				version, description,
				KAboutData::License_GPL, "(C) %{YEAR}, %{AUTHOR}");
	aboutdata.addAuthor("%{AUTHOR}",I18N_NOOP("Developer"),"%{EMAIL}");

	KCmdLineArgs::init( argc, argv, &aboutdata );
	KCmdLineArgs::addCmdLineOptions( options );
	KUniqueApplication::addCmdLineOptions();

	if (!KUniqueApplication::start())
	{
		kdDebug() << "%{APPNAMELC} is already running!" << endl;
		return (0);
	}

	KUniqueApplication app;
	kdDebug() << "starting %{APPNAMELC} " << endl;
	// This app is started automatically, no need for session management
	app.disableSessionManagement();
	%{APPNAME} *service = new %{APPNAME};
	kdDebug() << "starting %{APPNAMELC} " << endl;
	return app.exec();

}
