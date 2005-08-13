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

#include <qwhatsthis.h>

#include <kdevgenericfactory.h>
#include <kiconloader.h>
#include <klocale.h>

#include "kdevcore.h"
#include "kdevmainwindow.h"
#include "kdevplugininfo.h"

#include "konsoleviewwidget.h"


static const KDevPluginInfo data("kdevkonsoleview");
K_EXPORT_COMPONENT_FACTORY( libkdevkonsoleview, KDevGenericFactory<KonsoleViewPart>( data ) )

KonsoleViewPart::KonsoleViewPart(QObject *parent, const char *name, const QStringList &)
  : KDevPlugin(&data, parent, name ? name : "KonsoleViewPart")
{
    m_widget = new KonsoleViewWidget(this);

    QWhatsThis::add(m_widget, i18n("<b>Konsole</b><p>"
        "This window contains an embedded konsole window. It will try to follow you when "
        "you navigate in the source directories")
    );

    m_widget->setIcon( SmallIcon("konsole") );
    m_widget->setCaption(i18n("Konsole"));
    
    mainWindow()->embedOutputView(m_widget, i18n("Konsole"), i18n("Embedded console window"));
}


KonsoleViewPart::~KonsoleViewPart()
{
    if ( m_widget )
        mainWindow()->removeView( m_widget );
    delete m_widget;
}


#include "konsoleviewpart.moc"
