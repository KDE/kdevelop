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
#include <kdevgenericfactory.h>
#include <kdevplugininfo.h>
#include <kdebug.h>
#include <kiconloader.h>

#include <kdevcore.h>
#include <kdevmainwindow.h>

#include "replace_widget.h"
#include "replace_part.h"

static const KDevPluginInfo data("kdevreplace");

typedef KDevGenericFactory<ReplacePart> ReplaceFactory;
K_EXPORT_COMPONENT_FACTORY(libkdevreplace, ReplaceFactory(data))

ReplacePart::ReplacePart(QObject *parent, const char *name, const QStringList& )
        : KDevPlugin( &data, parent, name ? name : "ReplacePart" )
{
    setInstance(ReplaceFactory::instance());
    setXMLFile("kdevpart_replace.rc");

    m_widget = new ReplaceWidget(this);
    m_widget->setIcon( SmallIcon("filefind") );
    m_widget->setCaption(i18n("Replace"));
    
    QWhatsThis::add
        (m_widget, i18n("<b>Replace</b><p>"
                        "This window shows a preview of a string replace "
                        "operation. Uncheck a line to exclude that replacement. "
                        "Uncheck a file to exclude the whole file from the "
                        "operation. "
                        "Clicking on a line in the list will automatically "
                        "open the corresponding source file and set the "
                        "cursor to the line with the match." ));

    mainWindow()->embedOutputView( m_widget, i18n("Replace"), i18n("Project wide string replacement") );

    KAction * action = new KAction(i18n("Find-Select-Replace..."), "replace project",
                                   CTRL+SHIFT+Key_R, this, SLOT(slotReplace()), actionCollection(), "edit_replace_across");
    action->setToolTip( i18n("Project wide string replacement") );
    action->setWhatsThis( i18n("<b>Find-Select-Replace</b><p>"
                               "Opens the project wide string replacement dialog. There you "
                               "can enter a string or a regular expression which is then "
                               "searched for within all files in the locations "
                               "you specify. Matches will be displayed in the <b>Replace</b> window, you "
                               "can replace them with the specified string, exclude them from replace operation or cancel the whole replace.") );
}


ReplacePart::~ReplacePart()
{
    if ( m_widget )
        mainWindow()->removeView( m_widget );
    delete m_widget;
}

void ReplacePart::slotReplace()
{
    m_widget->showDialog();
}


#include "replace_part.moc"
