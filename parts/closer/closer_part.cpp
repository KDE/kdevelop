/***************************************************************************
 *   Copyright (C) 2003 by Jens Dagerbo                                    *
 *   jens.dagerbo@swipnet.se                                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qdialog.h>
#include <qstringlist.h>
#include <qstring.h>

#include <kiconloader.h>
#include <klocale.h>
#include <kgenericfactory.h>
#include <kaction.h>
#include <kdebug.h>
#include <kdevpartcontroller.h>
#include <kparts/part.h>
#include <kdevproject.h>
#include <kdevcore.h>
#include <kdevmainwindow.h>

#include "closer_part.h"
#include "closer_dialogimpl.h"


typedef KGenericFactory<CloserPart> CloserFactory;
K_EXPORT_COMPONENT_FACTORY( libkdevcloser, CloserFactory( "kdevcloser" ) )

CloserPart::CloserPart(QObject *parent, const char *name, const QStringList& )
    : KDevPlugin("Selected Window Closer", "closer", parent, name ? name : "closerPart" )
{
    setInstance(CloserFactory::instance());
    setXMLFile("kdevpart_closer.rc");

    KAction * action = new KAction( i18n("Close Selected Windows..."), CTRL+ALT+Key_W, this,
        SLOT( openDialog() ), actionCollection(), "closer" );
    action->setToolTip( i18n("Select windows to close") );
    action->setWhatsThis(i18n("<b>Close selected windows</b><p>Provides a dialog to select files which will be closed."));

    core()->insertNewAction( action );
}


CloserPart::~CloserPart()
{}

void CloserPart::openDialog()
{
    CloserDialogImpl d( openFiles() );
    if ( d.exec() == QDialog::Accepted )
    {
        closeFiles( d.getCheckedFiles() );
    }
}

KURL::List CloserPart::openFiles()
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

void CloserPart::closeFiles( KURL::List const & fileList )
{
    KURL::List::ConstIterator it = fileList.begin();
    while ( it != fileList.end() )
    {
        if ( KParts::ReadOnlyPart * ro_part = partForURL( *it ) )
        {
            ro_part->closeURL();
            partController()->removePart( ro_part );
            delete ro_part;
        }
        ++it;
    }
}

// reimplemented from PartController::partForURL to avoid linking
KParts::ReadOnlyPart * CloserPart::partForURL( KURL const & url )
{
    QPtrListIterator<KParts::Part> it( *partController()->parts() );
    while( it.current() )
    {
        KParts::ReadOnlyPart *ro_part = dynamic_cast<KParts::ReadOnlyPart*>(it.current());
        if (ro_part && url == ro_part->url())
        {
            return ro_part;
        }
        ++it;
    }
    return 0;
}
#include "closer_part.moc"
