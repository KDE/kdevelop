/***************************************************************************
 *   Copyright (C) 2003 by KDevelop Authors                                *
 *   kdevelop-devel@kde.org                                                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "konsoleviewpart.h"

#include <kdevgenericfactory.h>
#include <kicon.h>
#include <klocale.h>

#include "kdevcore.h"
#include "kdevmainwindow.h"
#include "kdevplugininfo.h"

#include "konsoleviewwidget.h"

static const KDevPluginInfo data("kdevkonsoleview");
K_EXPORT_COMPONENT_FACTORY( kdevkonsoleview, KDevGenericFactory<KonsoleViewPart>( data ) )

KonsoleViewPart::KonsoleViewPart(QObject *parent, const QStringList &)
  : KDevPlugin(&data, parent)
{
    m_widget = new KonsoleViewWidget(this);

    m_widget->setWhatsThis(i18n("<b>Konsole</b><p>"
                                "This window contains an embedded command line terminal."));

    m_widget->setWindowIcon( KIcon("konsole") );
    m_widget->setWindowTitle(i18n("Konsole"));

    KDevApi::self() ->mainWindow()->embedOutputView(m_widget, i18n("Konsole"), i18n("Embedded console window"));
}


KonsoleViewPart::~KonsoleViewPart()
{
    if ( m_widget )
        KDevApi::self() ->mainWindow()->removeView( m_widget );
}


#include "konsoleviewpart.moc"
