%{CPP_TEMPLATE}

//////////////////////////////////////////////////////////////////////
// KDE specific include files
#include <kdebug.h>
//////////////////////////////////////////////////////////////////////
// Application specific include files
#include "kbear%{APPNAMELC}outputwidget.h"


KBear%{APPNAME}OutputWidget::KBear%{APPNAME}OutputWidget(QWidget* parent, const char* name )
    : QWidget( parent, name )
{
}

KBear%{APPNAME}OutputWidget::~KBear%{APPNAME}OutputWidget() {
}


#include "kbear%{APPNAMELC}outputwidget.moc"

