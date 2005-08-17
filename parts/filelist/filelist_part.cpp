/***************************************************************************
 *   Copyright (C) 2004 by Jens Dagerbo                                    *
 *   jens.dagerbo@swipnet.se                                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <q3whatsthis.h>

#include <kiconloader.h>
#include <klocale.h>
#include <kdevgenericfactory.h>
#include <kdevpartcontroller.h>
#include <kurl.h>
#include <kparts/part.h>
#include <kdebug.h>

#include "kdevcore.h"
#include "kdevmainwindow.h"
#include "kdevplugininfo.h"

#include "filelist_widget.h"
#include "filelist_part.h"

static const KDevPluginInfo data("kdevfilelist");

typedef KDevGenericFactory<FileListPart> filelistFactory;
K_EXPORT_COMPONENT_FACTORY( libkdevfilelist, filelistFactory( data ) )

FileListPart::FileListPart(QObject *parent, const char *name, const QStringList& )
        : KDevPlugin(&data, parent, name ? name : "FileListPart" )
{
    setInstance(filelistFactory::instance());
    //setXMLFile("kdevpart_filelist.rc");

    m_widget = new FileListWidget(this);
	m_widget->setCaption(i18n("File List"));
	m_widget->setIcon( SmallIcon( info()->icon() ) );

    Q3WhatsThis::add
        (m_widget, i18n("<b>File List</b><p>This is the list of opened files."));

    mainWindow()->embedSelectView( m_widget, i18n("File List"), i18n("Open files") );
}


FileListPart::~FileListPart()
{
    if ( m_widget )
    {
        mainWindow()->removeView( m_widget );
    }
    delete m_widget;
}

#include "filelist_part.moc"

// kate: space-indent off; indent-width 4; tab-width 4; show-tabs off;
