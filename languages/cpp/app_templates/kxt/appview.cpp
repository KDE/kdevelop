
#include "$APPNAMELC$view.h"
#include "settings.h"

#include <klocale.h>
#include <qlabel.h>

$APPNAME$View::$APPNAME$View(QWidget *parent)
    : $APPNAME$view_base(parent)
{
	settingsChanged();
}

$APPNAME$View::~$APPNAME$View()
{

}

void $APPNAME$View::switchColors()
{
	// switch the foreground/background colors of the label
	QColor color = Settings::col_background();
	Settings::setCol_background( Settings::col_foreground() );
	Settings::setCol_foreground( color );

	settingsChanged();
}

void $APPNAME$View::settingsChanged()
{
	sillyLabel->setPaletteBackgroundColor( Settings::col_background() );
	sillyLabel->setPaletteForegroundColor( Settings::col_foreground() );

	// i18n : internationalization
	sillyLabel->setText( i18n("This project is %1 days old").arg(Settings::val_time()) );
	emit signalChangeStatusbar( i18n("Settings changed") );
}

#include "$APPNAMELC$view.moc"

