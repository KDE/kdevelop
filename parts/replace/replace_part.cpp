/***************************************************************************
 *   Copyright (C) 2003 by Jens Dagerbo                                    *
 *   jens@krypton.supernet                                                 *
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

#include <kdevcore.h>
#include <kdevmainwindow.h>

#include "replace_widget.h"
#include "replace_part.h"

typedef KGenericFactory<ReplacePart> ReplaceFactory;
K_EXPORT_COMPONENT_FACTORY( libkdevreplace, ReplaceFactory( "kdevreplace" ) );

ReplacePart::ReplacePart(QObject *parent, const char *name, const QStringList& )
        : KDevPlugin( "replace-across", "grep", parent, name ? name : "ReplacePart" )
{
    setInstance(ReplaceFactory::instance());
    setXMLFile("kdevpart_replace.rc");

    m_widget = new ReplaceWidget(this);

    QWhatsThis::add
        (m_widget, i18n("WHAT DOES THIS PART DO?"));

    mainWindow()->embedOutputView( m_widget, "Replace", "Replace strings across project" );

    KAction * action = new KAction(i18n("Re&place String Across Project..."), "replace project",
                                   CTRL+ALT+Key_P, this, SLOT(slotReplace()), actionCollection(), "edit_replace_across");
    action->setStatusText( i18n("Replaces a string across the projcect") );
    action->setWhatsThis( i18n("Replace across project\n\n"
                               "Bla, bla, bla....") );

    core()->insertNewAction( action );
}


ReplacePart::~ReplacePart()
{
    delete m_widget;
}

void ReplacePart::slotReplace()
{
    kdDebug(0) << " ******* ReplacePart::slotReplace()" << endl;
    m_widget->showDialog();
}


#include "replace_part.moc"
