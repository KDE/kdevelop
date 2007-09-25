%{CPP_TEMPLATE}
#include "%{APPNAMELC}view.h"
#include "settings.h"

#include <klocale.h>
#include <QtGui/QLabel>

%{APPNAME}View::%{APPNAME}View(QWidget *)
{
    ui_%{APPNAMELC}view_base.setupUi(this);
    settingsChanged();
    setAutoFillBackground(true);
}

%{APPNAME}View::~%{APPNAME}View()
{

}

void %{APPNAME}View::switchColors()
{
    // switch the foreground/background colors of the label
    QColor color = Settings::col_background();
    Settings::setCol_background( Settings::col_foreground() );
    Settings::setCol_foreground( color );

    settingsChanged();
}

void %{APPNAME}View::settingsChanged()
{
    QPalette pal;
    pal.setColor( QPalette::Window, Settings::col_background());
    pal.setColor( QPalette::WindowText, Settings::col_foreground());
    ui_%{APPNAMELC}view_base.kcfg_sillyLabel->setPalette( pal );

    // i18n : internationalization
    ui_%{APPNAMELC}view_base.kcfg_sillyLabel->setText( i18n("This project is %1 days old",Settings::val_time()) );
    emit signalChangeStatusbar( i18n("Settings changed") );
}

#include "%{APPNAMELC}view.moc"
