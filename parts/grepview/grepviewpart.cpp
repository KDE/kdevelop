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

#include <qpopupmenu.h>
#include <qvbox.h>
#include <qwhatsthis.h>
#include <kdebug.h>
#include <klocale.h>
#include <kaction.h>
#include <kdialogbase.h>
#include <kiconloader.h>

#include "kdevcore.h"
#include "kdevtoplevel.h"
#include "grepviewwidget.h"


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
    connect( core(), SIGNAL(contextMenu(QPopupMenu *, const Context *)),
             this, SLOT(contextMenu(QPopupMenu *, const Context *)) );

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
    
    action = new KAction(i18n("&Grep..."), "grep", CTRL+ALT+Key_F,
                         this, SLOT(slotGrep()),
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


void GrepViewPart::contextMenu(QPopupMenu *popup, const Context *context)
{
    kdDebug(9001) << "context in grepview" << endl;
    if (!context->hasType("editor"))
        return;
    
    const EditorContext *econtext = static_cast<const EditorContext*>(context);
    QString ident = econtext->currentWord();
    if (!ident.isEmpty()) {
        m_popupstr = ident;
        popup->insertItem( i18n("Grep: %1").arg(ident),
                           this, SLOT(slotContextGrep()) );
    }
}


void GrepViewPart::slotGrep()
{
    m_widget->showDialog();
}


void GrepViewPart::slotContextGrep()
{
    m_widget->showDialogWithPattern(m_popupstr);
}

#include "grepviewpart.moc"
