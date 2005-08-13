%{CPP_TEMPLATE}

// Qt specific include files
#include <qstring.h>

// KDE specific include files
#include <klocale.h>
#include <kdebug.h>

// Application specific include files
#include "kbear%{APPNAMELC}configwidget.h"
#include "kbear%{APPNAMELC}configwidget.moc"

using namespace KBear;

KBear%{APPNAME}ConfigWidget::KBear%{APPNAME}ConfigWidget( QWidget* parent, const char* name )
    : KBearConfigWidgetIface( parent, name )
{
    readSettings();
}

KBear%{APPNAME}ConfigWidget::~KBear%{APPNAME}ConfigWidget()
{
}

void KBear%{APPNAME}ConfigWidget::slotSaveSettings()
{
    saveSettings();
}

void KBear%{APPNAME}ConfigWidget::saveSettings()
{
}

void KBear%{APPNAME}ConfigWidget::readSettings( bool defaultSettings )
{
}

