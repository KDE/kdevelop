/*
       This file is part of Kontact
       Copyright (c) 2002 Daniel Molkentin <molkentin@kde.org>
       
       This library is free software; you can redistribute it and/or
       modify it under the terms of the GNU Library General Public
       License as published by the Free Software Foundation; either
       version 2 of the License, or (at your option) any later version.
       
	   This library is distributed in the hope that it will be useful,
	   but WITHOUT ANY WARRANTY; without even the implied warranty of
	   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	   Library General Public License for more details.
	   
	   You should have received a copy of the GNU Library General Public License
	   along with this library; see the file COPYING.LIB.  If not, write to
	   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
	   Boston, MA 02111-1307, USA.
*/

#include <kmessagebox.h>
#include <kaction.h>
#include <kgenericfactory.h>
#include <kiconloader.h>
#include <kstatusbar.h>

#include <kdevcore.h>
#include <kdevpartcontroller.h>

#include <kdebug.h>

#include "knotes_plugin.h"
#include "knotes_part.h"

typedef KGenericFactory<KNotesPlugin> KNotesPluginFactory;
K_EXPORT_COMPONENT_FACTORY( libkdevnotes, KNotesPluginFactory( "kdevnotes" ) );

KNotesPlugin::KNotesPlugin( QObject* parent, const char *name, const QStringList &)
    : KDevPlugin(i18n("Notes"), "knotes", parent, name ? name : "KDevNotes" ), m_part(0)
{
    setInstance(KNotesPluginFactory::instance());

    setXMLFile("kdevpart_notes.rc");

    core()->insertNewAction(new KAction(i18n("New Note"), BarIcon("knotes"), 0, this, SLOT(slotNewNote()), actionCollection(), "new_note" ) );

    connect( partController(), SIGNAL(partRemoved(KParts::Part*)),
	     this, SLOT(slotPartRemoved(KParts::Part*)) );

    (void) new KAction(i18n("Show Notes"), BarIcon("knotes"), 0, this, SLOT(slotShowNotes()), actionCollection(), "show_notes" );
    // showPart();
}

KNotesPlugin::~KNotesPlugin()
{
}

KParts::Part* KNotesPlugin::part()
{
    if (!m_part){
	m_part = new KNotesPart(this, "notes");
    }

    return m_part;
}

void KNotesPlugin::slotNewNote()
{
    showPart();
    if ( m_part )
	m_part->slotNewNote();
}

void KNotesPlugin::slotShowNotes()
{
    showPart();
}

void KNotesPlugin::slotPartRemoved( KParts::Part* part )
{
    if( part == m_part ){
	m_part = 0;
    }
}

#include "knotes_plugin.moc"
