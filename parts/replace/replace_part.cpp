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

#include <qwhatsthis.h>

#include <kaction.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kgenericfactory.h>
#include <kdebug.h>
#include <kiconloader.h>

#include <kdevcore.h>
#include <kdevmainwindow.h>

#include "replace_widget.h"
#include "replace_part.h"

typedef KGenericFactory<ReplacePart> ReplaceFactory;
K_EXPORT_COMPONENT_FACTORY( libkdevreplace, ReplaceFactory( "kdevreplace" ) )

ReplacePart::ReplacePart(QObject *parent, const char *name, const QStringList& )
        : KDevPlugin( "Project Wide Replace", "", parent, name ? name : "ReplacePart" )
{
    setInstance(ReplaceFactory::instance());
    setXMLFile("kdevpart_replace.rc");

    m_widget = new ReplaceWidget(this);
    m_widget->setIcon( SmallIcon("filefind") );

    QWhatsThis::add
        (m_widget, i18n("Replace\n\n"
                        "This window shows a preview of a string replace "
                        "operation. Uncheck a line to exclude that replacement. "
                        "Uncheck a file to exclude the whole file from the "
                        "operation. "
                        "Clicking on a line in the list will automatically "
                        "open the corresponding source file and set the "
                        "cursor to the line with the match." ));

    mainWindow()->embedOutputView( m_widget, i18n("Replace"), i18n("project wide string replacement") );

    KAction * action = new KAction(i18n("Find-Select-Replace..."), "replace project",
                                   CTRL+ALT+Key_R, this, SLOT(slotReplace()), actionCollection(), "edit_replace_across");
    action->setStatusText( i18n("Replaces a string across the project") );
    action->setWhatsThis( i18n("Replaces a string across the project") );

    core()->insertNewAction( action );
}


ReplacePart::~ReplacePart()
{
    delete m_widget;
}

void ReplacePart::slotReplace()
{
    m_widget->showDialog();
}


#include "replace_part.moc"
