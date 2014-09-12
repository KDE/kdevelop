/*
 * kde4appview.cpp
 *
 * Copyright (C) 2008 %{AUTHOR} <%{EMAIL}>
 */
#include "kde4appview.h"
#include "settings.h"

#include <KLocalizedString>
#include <QtGui/QLabel>

kde4appView::kde4appView(QWidget *)
{
    ui_kde4appview_base.setupUi(this);
    settingsChanged();
    setAutoFillBackground(true);
}

kde4appView::~kde4appView()
{

}

void kde4appView::switchColors()
{
    // switch the foreground/background colors of the label
    QColor color = Settings::col_background();
    Settings::setCol_background( Settings::col_foreground() );
    Settings::setCol_foreground( color );

    settingsChanged();
}

void kde4appView::settingsChanged()
{
    QPalette pal;
    pal.setColor( QPalette::Window, Settings::col_background());
    pal.setColor( QPalette::WindowText, Settings::col_foreground());
    ui_kde4appview_base.kcfg_sillyLabel->setPalette( pal );

    // i18np : internationalization
    ui_kde4appview_base.kcfg_sillyLabel->setText( i18np("This project is one day old", "This project is %1 days old", Settings::val_time()) );
    emit signalChangeStatusbar( i18n("Settings changed") );
}

#include "kde4appview.moc"
