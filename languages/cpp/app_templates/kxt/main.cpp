
#include "%{APPNAMELC}.h"
#include <kapplication.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <klocale.h>

static const char description[] =
    I18N_NOOP("A KDE Application");

static const char version[] = "%{VERSION}";

static KCmdLineOptions options[] =
{
    { "+[URL]", I18N_NOOP( "Document to open." ), 0 },
    KCmdLineLastOption
};

int main(int argc, char **argv)
{
    KAboutData about("%{APPNAMELC}", I18N_NOOP("%{APPNAME}"), version, description,
                     KAboutData::License_$LICENSE$, "(C) %{YEAR} %{AUTHOR}", 0, 0, "%{EMAIL}");
    about.addAuthor( "%{AUTHOR}", 0, "%{EMAIL}" );
    KCmdLineArgs::init(argc, argv, &about);
    KCmdLineArgs::addCmdLineOptions(options);
    KApplication app;

    // see if we are starting with session management
    if (app.isRestored())
    {
        RESTORE(%{APPNAME});
    }
    else
    {
        // no session.. just start up normally
        KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
        if (args->count() == 0)
        {
            %{APPNAME} *widget = new %{APPNAME};
            widget->show();
        }
        else
        {
            int i = 0;
            for (; i < args->count(); i++)
            {
                %{APPNAME} *widget = new %{APPNAME};
                widget->show();
            }
        }
        args->clear();
    }

    return app.exec();
}

