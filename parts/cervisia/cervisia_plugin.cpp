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

#include <qvbox.h>
#include <qtextedit.h>
#include <klistview.h>

#include <kinstance.h>
#include <klocale.h>
#include <kgenericfactory.h>
#include <kdebug.h>
#include <kaction.h>
#include <kurl.h>
#include <kmessagebox.h>
#include <kmainwindow.h>

#include <kparts/part.h>
#include <klibloader.h>
#include <kparts/componentfactory.h>

#include <kdevmainwindow.h>
#include <kdevcore.h>
#include <kdevproject.h>

#include "cervisia_plugin.h"

typedef KGenericFactory<CervisiaPlugin> CervisiaPluginFactory;
K_EXPORT_COMPONENT_FACTORY( libkdevcervisia, CervisiaPluginFactory( "kdevcervisia" ) );

CervisiaPlugin::CervisiaPlugin(  QObject *parent, const char *name, const QStringList &arguments )
    : KDevVersionControl( "CervisiaPlugin", "cervisiaplugin", parent, name ? name : "CervisiaPlugin" ),
    m_part( 0 )
{
    kdDebug() << "CervisiaPlugin::CervisiaPlugin()" << endl;
    // we need an instance
    setInstance( CervisiaPluginFactory::instance() );

//    setXMLFile( "kdevcervisia.rc" );

    m_part = KParts::ComponentFactory::createPartInstanceFromLibrary<KParts::ReadOnlyPart>(
        "libcervisiapart", 0, "cervisiapart", this, 0, arguments );
    if (!m_part)
    {
        // @todo fix this brutal return
        KMessageBox::error( mainWindow()->main()->centralWidget(),
            i18n("Could not find Cervisia KPart!! This plug-in will be unusable.") );
        return;
    }

    // Hmmmm --- try to pilot cervisia opening the sandbox when starting ...
    connect( core(), SIGNAL(projectOpened()), this, SLOT(slotProjectOpened()) );
    connect( core(), SIGNAL(projectClosed()), this, SLOT(slotProjectClosed()) );
}

CervisiaPlugin::~CervisiaPlugin()
{
    if (m_part)
    {
        mainWindow()->removeView( m_part->widget() );
        mainWindow()->main()->guiFactory()->removeClient( m_part );
        m_part->closeURL();
        delete m_part;
    }
}

void CervisiaPlugin::slotProjectOpened()
{
    if (!m_part)
        return;

    // Embed cervisia GUI
    mainWindow()->main()->guiFactory()->addClient( m_part );
    mainWindow()->embedSelectView( m_part->widget(), i18n("Cervisia"), i18n("Provides CVS Access"));

    KURL projectUrl = KURL::fromPathOrURL( project()->projectDirectory() );
    m_part->openURL( projectUrl );
}

void CervisiaPlugin::slotProjectClosed()
{
    if (!m_part)
        return;

    // Remove the KPart's GUI since I see no way on how to tell to cervisiapart "deactivate!" ;-)
    m_part->closeURL();
    mainWindow()->removeView( m_part->widget() );
    mainWindow()->main()->guiFactory()->removeClient( m_part );
}

#include "cervisia_plugin.moc"
