%{CPP_TEMPLATE}

#include "%{APPNAMELC}view.h"
#include "settings.h"

#include <klocale.h>
#include <qlabel.h>

%{APPNAMELC}View::%{APPNAMELC}View(QWidget *parent)
    : %{APPNAMELC}view_base(parent)
{
	settingsChanged();
}

%{APPNAMELC}View::~%{APPNAMELC}View()
{

}

void %{APPNAMELC}View::switchColors()
{
	// switch the foreground/background colors of the label
	QColor color = Settings::col_background();
	Settings::setCol_background( Settings::col_foreground() );
	Settings::setCol_foreground( color );

	settingsChanged();
}

void %{APPNAMELC}View::settingsChanged()
{
	sillyLabel->setPaletteBackgroundColor( Settings::col_background() );
	sillyLabel->setPaletteForegroundColor( Settings::col_foreground() );

	// i18n : internationalization
	sillyLabel->setText( i18n("This project is %1 days old", Settings::val_time()) );
	emit signalChangeStatusbar( i18n("Settings changed") );
}

#include "%{APPNAMELC}view.moc"

