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
#include <kgenericfactory.h>
#include <kdebug.h>

#include <kmainwindow.h>
#include "kdevmainwindow.h"

#include "partexplorerform.h"

typedef KGenericFactory<PartExplorerPlugin> PartExplorerPluginFactory;
K_EXPORT_COMPONENT_FACTORY( libkdevpartexplorer, PartExplorerPluginFactory( "kdevpartexplorer" ) );

PartExplorerPlugin::PartExplorerPlugin(  QObject *parent, const char *name, const QStringList & )
    : KDevPlugin( "PartExplorer", "partexplorer", parent, name ? name : "PartExplorerPlugin" )
{
    // we need an instance
    setInstance( PartExplorerPluginFactory::instance() );

    // this should be your custom internal widget
    m_widget = new PartExplorerForm();
    mainWindow()->embedSelectView( m_widget, i18n("PartExplorer"), i18n("Query system services"));
}

PartExplorerPlugin::~PartExplorerPlugin()
{
    if (m_widget)
    {
        mainWindow()->removeView( m_widget );
    }
    delete m_widget;
}

#include "partexplorer_plugin.moc"
