/***************************************************************************
 *   Copyright (C) 1999-2001 by Bernd Gehrmann                             *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "grepviewpart.h"

#include <qvbox.h>
#include <qwhatsthis.h>
#include <kdebug.h>
#include <klocale.h>
#include <kaction.h>
#include <kdialogbase.h>
#include <kgenericfactory.h>
#include <kiconloader.h>

#include "kdevcore.h"
#include "kdevtoplevel.h"
#include "grepviewwidget.h"


typedef KGenericFactory<GrepViewPart> GrepViewFactory;
K_EXPORT_COMPONENT_FACTORY( libkdevgrepview, GrepViewFactory( "kdevgrepview" ) );

GrepViewPart::GrepViewPart( QObject *parent, const char *name, const QStringList & )
    : KDevPlugin( parent, name )
{
    setInstance(GrepViewFactory::instance());
    
    setXMLFile("kdevgrepview.rc");

    connect( core(), SIGNAL(stopButtonClicked()),
             this, SLOT(stopButtonClicked()) );
    connect( core(), SIGNAL(projectOpened()), this, SLOT(projectOpened()) );
    connect( core(), SIGNAL(projectClosed()), this, SLOT(projectClosed()) );

    m_widget = new GrepViewWidget(this);
    m_widget->setIcon(SmallIcon("grep"));
    m_widget->setCaption(i18n("Grep Output"));
    QWhatsThis::add(m_widget, i18n("Grep\n\n"
                                   "This window contains the output of a grep "
                                   "command. Clicking on an item in the list "
                                   "will automatically open the corresponding "
                                   "source file and set the cursor to the line "
                                   "with the match."));

    topLevel()->embedOutputView(m_widget, i18n("Grep"));
    
    KAction *action;
    
    action = new KAction(i18n("&Grep"), "grep", CTRL+ALT+Key_F,
                         m_widget, SLOT(showDialog()),
                         actionCollection(), "edit_grep");
    action->setStatusText( i18n("Searches for expressions over several files") );
    action->setWhatsThis( i18n("Search in files\n\n"
                               "Opens the Search in files dialog. There you "
                               "can enter a regular expression which is then "
                               "searched through all files in the directories "
                               "you specify. Matches will be displayed, you "
                               "can switch to a match directly.") );
}


GrepViewPart::~GrepViewPart()
{
    delete m_widget;
}


void GrepViewPart::slotRaiseWidget()
{
    topLevel()->raiseView(m_widget);
}


void GrepViewPart::stopButtonClicked()
{
    kdDebug(9001) << "GrepViewPart::stopButtonClicked()" << endl;
    m_widget->killJob();
}


void GrepViewPart::projectOpened()
{
    kdDebug(9001) << "GrepViewPart::projectOpened()" << endl;
    m_widget->projectChanged(project());
}


void GrepViewPart::projectClosed()
{
    m_widget->projectChanged(0);
}

#include "grepviewpart.moc"
