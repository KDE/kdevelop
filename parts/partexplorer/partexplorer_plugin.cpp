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
#include <kaction.h>
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
    // set our XML-UI resource file
    setXMLFile( "kdevpartexplorer.rc" );

    KAction *action;
    // This plugin offer only one action, a menu entry for showing the PartExplorerForm ...
    action = new KAction( i18n("Part Explorer"), 0, this, SLOT(slotShowWidget()),
        actionCollection(), "show_partexplorerform" );

    // this should be your custom internal widget
    m_widget = new PartExplorerForm( mainWindow()->main()->centralWidget() );
    m_widget->hide();

    // When the user press "Search" button then this part performs the query.
    connect(
        m_widget, SIGNAL(searchQuery(QString,QString)),
        this, SLOT(slotSearchServices(QString,QString))
    );
    // Let the widget to display any error message we may encounter.
    connect(
        this, SIGNAL(displayError(QString)),
        m_widget, SLOT(slotDisplayError(QString))
    );
}

PartExplorerPlugin::~PartExplorerPlugin()
{
//    delete m_widget;
}

void PartExplorerPlugin::slotShowWidget()
{
    m_widget->show();
}

#include "partexplorer_plugin.moc"
