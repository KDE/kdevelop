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
#include <kprocess.h>
#include <kstringhandler.h>

#include "kdevcore.h"
#include "kdevmainwindow.h"
#include "grepviewwidget.h"

static const KAboutData data("kdevgrepview", I18N_NOOP("Grep"), "1.0");

//typedef KDevGenericFactory<GrepViewPart> GrepViewFactory;

K_EXPORT_COMPONENT_FACTORY(libkdevgrepview, GrepViewFactory(&data))

//K_EXPORT_COMPONENT_FACTORY( libkdevgrepview, GrepViewFactory( "kdevgrepview" ) )

GrepViewPart::GrepViewPart( QObject *parent, const char *name, const QStringList & )
    : KDevPlugin( "GrepView", "grep", parent, name ? name : "GrepViewPart" )
{
    setInstance(GrepViewFactory::instance());

    setXMLFile("kdevgrepview.rc");

    connect( core(), SIGNAL(stopButtonClicked(KDevPlugin*)),
             this, SLOT(stopButtonClicked(KDevPlugin*)) );
    connect( core(), SIGNAL(projectOpened()), this, SLOT(projectOpened()) );
    connect( core(), SIGNAL(projectClosed()), this, SLOT(projectClosed()) );
    connect( core(), SIGNAL(contextMenu(QPopupMenu *, const Context *)),
             this, SLOT(contextMenu(QPopupMenu *, const Context *)) );

    m_widget = new GrepViewWidget(this);
    m_widget->setIcon(SmallIcon("find"));
    m_widget->setCaption(i18n("Grep Output"));
    QWhatsThis::add(m_widget, i18n("<b>Find in files</b><p>"
                                   "This window contains the output of a grep "
                                   "command. Clicking on an item in the list "
                                   "will automatically open the corresponding "
                                   "source file and set the cursor to the line "
                                   "with the match."));

    mainWindow()->embedOutputView(m_widget, i18n("Find in files"), i18n("Output of the grep command"));

    KAction *action;

    action = new KAction(i18n("Find in Fi&les..."), "grep", CTRL+ALT+Key_F,
                         this, SLOT(slotGrep()),
                         actionCollection(), "edit_grep");
    action->setToolTip( i18n("Search for expressions over several files") );
    action->setWhatsThis( i18n("<b>Find in files</b><p>"
                               "Opens the 'Find in files' dialog. There you "
                               "can enter a regular expression which is then "
                               "searched for within all files in the directories "
                               "you specify. Matches will be displayed, you "
                               "can switch to a match directly.") );
}


GrepViewPart::~GrepViewPart()
{
	if ( m_widget )
		mainWindow()->removeView( m_widget );
    delete m_widget;
}


void GrepViewPart::stopButtonClicked(KDevPlugin* which)
{
    if ( which != 0 && which != this )
        return;
    kdDebug(9001) << "GrepViewPart::stopButtonClicked()" << endl;
    m_widget->killJob( SIGHUP );
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
    if (!context->hasType( Context::EditorContext ))
        return;

    const EditorContext *econtext = static_cast<const EditorContext*>(context);
    QString ident = econtext->currentWord();
    if (!ident.isEmpty()) {
        m_popupstr = ident;
        QString squeezed = KStringHandler::csqueeze(ident, 30);
        int id = popup->insertItem( i18n("Grep: %1").arg(squeezed),
                           this, SLOT(slotContextGrep()) );
        popup->setWhatsThis(id, i18n("<b>Grep</b><p>Opens the find in files dialog "
                               "and sets the pattern to the text under the cursor."));
        popup->insertSeparator();
    }
}


void GrepViewPart::slotGrep()
{
	if ( !m_widget->isRunning() )
	{
		m_widget->showDialog();
	}
}


void GrepViewPart::slotContextGrep()
{
	if ( !m_widget->isRunning() )
	{
		m_widget->showDialogWithPattern(m_popupstr);
	}
}

#include "grepviewpart.moc"
