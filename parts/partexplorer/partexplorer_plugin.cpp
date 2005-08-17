/***************************************************************************
 *   Copyright (C) 2003 by Mario Scalas                                    *
 *   mario.scalas@libero.it                                                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "partexplorer_plugin.h"

#include <kinstance.h>
#include <klocale.h>
#include <kdevgenericfactory.h>
#include <kdebug.h>
#include <kaction.h>
#include <kmainwindow.h>

#include <kdevplugininfo.h>
#include <kdevmainwindow.h>

#include "partexplorerform.h"

static const KDevPluginInfo data("kdevpartexplorer");

typedef KDevGenericFactory<PartExplorerPlugin> PartExplorerPluginFactory;
K_EXPORT_COMPONENT_FACTORY( libkdevpartexplorer, PartExplorerPluginFactory( data ) )

PartExplorerPlugin::PartExplorerPlugin(  QObject *parent, const char *name, const QStringList & )
    : KDevPlugin( &data, parent )
{
    setObjectName(name ? name : "PartExplorerPlugin");
    // we need an instance
    setInstance( PartExplorerPluginFactory::instance() );

    setXMLFile( "kdevpartexplorer.rc" );

    // this should be your custom internal widget
    m_widget = new PartExplorerForm( mainWindow()->main() );
//    mainWindow()->embedSelectView( m_widget, i18n("PartExplorer"), i18n("Query system services"));

    KAction *action = new KAction( i18n("&Part Explorer"), 0, this, SLOT(slotShowForm()),
        actionCollection(), "show_partexplorerform" );
    action->setToolTip(i18n("KTrader query execution"));
    action->setWhatsThis(i18n("<b>Part explorer</b><p>Shows a dialog for KTrader query execution. Search your KDE documentation for more information about KDE services and KTrader."));
}

PartExplorerPlugin::~PartExplorerPlugin()
{
/*
    if (m_widget)
    {
        mainWindow()->removeView( m_widget );
    }
*/	
    //delete m_widget;
}

void PartExplorerPlugin::slotShowForm()
{
    m_widget->show();
}

#include "partexplorer_plugin.moc"
