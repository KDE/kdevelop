
#include <kapplication.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <klocale.h>
#include <mainviewimp.h>

static const char description[] =    I18N_NOOP("A KDE Application");

static const char version[] = "%{VERSION}";

static KCmdLineOptions options[] =
    {
        { "+[URL]", I18N_NOOP( "Document to open" ), 0 },
        KCmdLineLastOption
    };


int main(int argc, char **argv)
{
	KAboutData about("%{APPNAME}", I18N_NOOP("%{APPNAME}"), version, description,
	                 KAboutData::License_%{LICENSE}, "(C) 2004 %{AUTHORNAME}", 0, 0, "%{EMAIL}");
	about.addAuthor( "%{AUTHORNAME}", 0, "%{EMAIL}" );
	KCmdLineArgs::init(argc, argv, &about);
	KCmdLineArgs::addCmdLineOptions(options);
	KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
	KApplication app;
	MainViewImp *widget = new MainViewImp;
	app.setMainWidget(widget);
	widget->show();
	return app.exec();
}

