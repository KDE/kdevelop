#include "konsoleviewpart.h"

#include <qwhatsthis.h>

#include <kgenericfactory.h>
#include <kiconloader.h>
#include <klocale.h>

#include "kdevcore.h"
#include "kdevmainwindow.h"

#include "konsoleviewwidget.h"


K_EXPORT_COMPONENT_FACTORY( libkdevkonsoleview, KGenericFactory<KonsoleViewPart>( "kdevkonsoleview" ) );

KonsoleViewPart::KonsoleViewPart(QObject *parent, const char *name, const QStringList &)
  : KDevPlugin("Konsole", "konsole", parent, name ? name : "KonsoleViewPart")
{
    m_widget = new KonsoleViewWidget(this);

    QWhatsThis::add(m_widget, i18n("Konsole\n\n"
        "This window contains an embedded konsole window. It will try to follow you when "
        "you navigate in the source directories")
    );

    mainWindow()->embedOutputView(m_widget, i18n("Konsole"), i18n("embedded console window"));
}


KonsoleViewPart::~KonsoleViewPart()
{
    delete m_widget;
}


#include "konsoleviewpart.moc"
