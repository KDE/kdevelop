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
#include <kdevgenericfactory.h>
#include <kaction.h>
#include <kdebug.h>
#include <kdevpartcontroller.h>
#include <kparts/part.h>
#include <kdevproject.h>
#include <kdevcore.h>
#include <kdevmainwindow.h>

#include "closer_part.h"
#include "closer_dialogimpl.h"

static const KAboutData data("kdevcloser", I18N_NOOP("Close Selected Windows..."), "1.0");

typedef KDevGenericFactory<CloserPart> CloserFactory;
K_EXPORT_COMPONENT_FACTORY( libkdevcloser, CloserFactory( &data ) )

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
    CloserDialogImpl d( partController()->openURLs() );
    if ( d.exec() == QDialog::Accepted )
    {
        partController()->closeFiles( d.getCheckedFiles() );
    }
}
#include "closer_part.moc"
