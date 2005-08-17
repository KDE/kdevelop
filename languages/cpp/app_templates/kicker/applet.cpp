//Added by qt3to4:
#include <QResizeEvent>
%{CPP_TEMPLATE}

#include <qlcdnumber.h>
#include <kglobal.h>
#include <klocale.h>
#include <kconfig.h>
#include <kapplication.h>
#include <kmessagebox.h>

#include "%{APPNAMELC}.h"


%{APPNAME}::%{APPNAME}(const QString& configFile, Type type, int actions, QWidget *parent, const char *name)
    : KPanelApplet(configFile, type, actions, parent, name)
{
    // Get the current application configuration handle
    ksConfig = config();
    QLCDNumber *w = new QLCDNumber(this);
    w->display(42);

    mainView = w;
    mainView->show();
}


%{APPNAME}::~%{APPNAME}()
{
}


void %{APPNAME}::about()
{
    KMessageBox::information(0, i18n("This is an about box"));
}


void %{APPNAME}::help()
{
    KMessageBox::information(0, i18n("This is a help box"));
}


void %{APPNAME}::preferences()
{
    KMessageBox::information(0, i18n("This is a preferences box"));
}

int %{APPNAME}::widthForHeight(int height) const
{
    return width();
}

int %{APPNAME}::heightForWidth(int width) const
{
    return height();
}

void %{APPNAME}::resizeEvent(QResizeEvent *e)
{
}


extern "C"
{
    KPanelApplet* init( QWidget *parent, const QString& configFile)
    {
        KGlobal::locale()->insertCatalogue("%{APPNAMELC}");
        return new %{APPNAME}(configFile, KPanelApplet::Normal,
                             KPanelApplet::About | KPanelApplet::Help | KPanelApplet::Preferences,
                             parent, "%{APPNAMELC}");
    }
}
