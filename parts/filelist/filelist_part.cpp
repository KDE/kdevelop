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

#include <qwhatsthis.h>

#include <kiconloader.h>
#include <klocale.h>
#include <kdevgenericfactory.h>
#include <kdevpartcontroller.h>
#include <kurl.h>
#include <kparts/part.h>
#include <kdebug.h>

#include "kdevcore.h"
#include "kdevmainwindow.h"


#include "filelist_widget.h"
#include "filelist_part.h"

static const KAboutData data("kdevfilelist", I18N_NOOP("File List"), "1.0");

typedef KDevGenericFactory<FileListPart> filelistFactory;
K_EXPORT_COMPONENT_FACTORY( libkdevfilelist, filelistFactory( &data ) )

FileListPart::FileListPart(QObject *parent, const char *name, const QStringList& )
        : KDevPlugin("filelist", "filelist", parent, name ? name : "FileListPart" )
{
    setInstance(filelistFactory::instance());
    //setXMLFile("kdevpart_filelist.rc");

    m_widget = new FileListWidget(this);
	m_widget->setCaption(i18n("File List"));

    QWhatsThis::add
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

// @todo remove. use partcontroller's method instead
KURL::List FileListPart::openFiles()
{
    KURL::List openfiles;
    if( const QPtrList<KParts::Part> * partlist = partController()->parts() )
    {
        QPtrListIterator<KParts::Part> it( *partlist );
        while ( KParts::Part* part = it.current() )
        {
            if ( KParts::ReadOnlyPart * ro_part = dynamic_cast<KParts::ReadOnlyPart*>( part ) )
            {
                openfiles.append( ro_part->url() );
            }
            ++it;
        }
    }
    return openfiles;
}

#include "filelist_part.moc"

// kate: space-indent off; indent-width 4; tab-width 4; show-tabs off;
