/* This file is part of the KDE project
   Copyright (C) 2002 Daniel Molkentin <molkentin@kde.org>
   
   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <qpopupmenu.h>

#include <kiconloader.h>
#include <kiconview.h>
#include <dcopclient.h>
#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>
#include <krun.h>
#include <kstandarddirs.h>
#include <kmessagebox.h>

#include "knotes_part.h"

class NotesItem : public KIconViewItem
{
public:
    NotesItem(KIconView * parent, const QString& id, const QString& text); 
    QString id() { return noteID; };
private:
    QString noteID;
};

NotesItem::NotesItem(KIconView * parent, const QString& id, const QString& text):
    KIconViewItem(parent, text, DesktopIcon("knotes"))
{
    noteID = id;
    setRenameEnabled(true);
}

KNotesPart::KNotesPart(QObject *parent, const char *name)
    : KParts::ReadOnlyPart(parent, name), 
    m_iconView(new KIconView), m_popupMenu(new QPopupMenu)
{
    
    m_popupMenu->insertItem(BarIcon("editdelete"), i18n("Remove Note"), 
			    this, SLOT(slotRemoveCurrentNote()));
    m_popupMenu->insertItem(BarIcon("editrename"), i18n("Rename Note"), 
			    this, SLOT(slotRenameCurrentNote()));
    
    connect(m_iconView, SIGNAL(executed(QIconViewItem*)), 
	    this, SLOT(slotOpenNote(QIconViewItem*)));
    connect(m_iconView, SIGNAL(rightButtonClicked(QIconViewItem*, const QPoint&)), 
	    this, SLOT(slotPopupRMB(QIconViewItem*, const QPoint&)));
    connect(m_iconView, SIGNAL(itemRenamed(QIconViewItem*, const QString&)),
	    this, SLOT(slotNoteRenamed(QIconViewItem*, const QString&)));
    
    initKNotes();
    setWidget(m_iconView);
    
    m_iconView->arrangeItemsInGrid();
    m_iconView->setItemsMovable(false);
}

void KNotesPart::initKNotes()
{
    QString *error = 0;
    int started = KApplication::startServiceByDesktopName("knotes", QString(), error);
    
    if ( started > 0 )
    {
	if (error) 
	    KMessageBox::error(0L, *error, i18n("Error"));
	return;
    }
    
    delete error;
    
    m_iconView->clear();
    
    NotesMap map;
    map = fetchNotes();
    NotesMap::const_iterator it;
    for (it = map.begin(); it != map.end(); ++it )
    {
	(void) new NotesItem( m_iconView, it.key(), it.data() );
    }
    
}

bool KNotesPart::openFile()
{
    return false;
}

NotesMap KNotesPart::fetchNotes()
{
    QCString replyType;
    QByteArray data, replyData;
    QDataStream arg(  data, IO_WriteOnly );
    if( kapp->dcopClient()->call( "knotes", "KNotesIface", "notes()", data, replyType, replyData ) )
    {
	kdDebug() << "Reply Type: " << replyType << endl;
	QDataStream answer(  replyData, IO_ReadOnly );
	NotesMap notes;
	answer >> notes;
	return notes;
    }
    else 
	return NotesMap();
    
}

void KNotesPart::slotPopupRMB(QIconViewItem *item, const QPoint& pos)
{
    if (!item) 
	return;
    
    m_popupMenu->popup(pos);
}

void KNotesPart::slotRemoveCurrentNote()
{
    QIconViewItem* item = m_iconView->currentItem();
    
    // better safe than sorry
    if (!item)
	return;
    
    QString id = static_cast<NotesItem*>( item )->id();
    
    QByteArray data;
    QDataStream arg( data, IO_WriteOnly );
    arg << id;
    if ( kapp->dcopClient()->send( "knotes", "KNotesIface", "killNote(QString)", data ) )
	kdDebug() << "Deleting Note!" << endl;
    
    // reinit knotes and refetch notes
    initKNotes();
}

void KNotesPart::slotRenameCurrentNote()
{
    // better safe than sorry
    if(m_iconView->currentItem()) 
	m_iconView->currentItem()->rename();
}


void KNotesPart::slotNoteRenamed(QIconViewItem *item, const QString& text)
{
    // better safe than sorry
    if (!item)
	return;
    
    QString id = static_cast<NotesItem*>( item )->id();
    
    QByteArray data;
    QDataStream arg( data, IO_WriteOnly );
    arg << id;
    arg << text;
    if ( kapp->dcopClient()->send( "knotes", "KNotesIface", "setName(QString, QString)", data ) )
	kdDebug() << "Rename Note!" << endl;
    
    // reinit knotes and refetch notes
    initKNotes();
    
    m_iconView->arrangeItemsInGrid();
}

void KNotesPart::slotOpenNote( QIconViewItem *item )
{
    QString id = static_cast<NotesItem*>( item )->id();
    
    QByteArray data;
    QDataStream arg( data, IO_WriteOnly );
    arg << id;
    if ( kapp->dcopClient()->send( "knotes", "KNotesIface", "showNote(QString)", data ) )
	kdDebug() << "Opening Note!" << endl;
}

void KNotesPart::slotNewNote()
{
    kdDebug() << "slotNewNote called!" << endl;
    QByteArray data;
    QDataStream arg(  data, IO_WriteOnly );
    arg << QString::null << QString::null;
    if ( !kapp->dcopClient()->send(  "knotes", "KNotesIface", "newNote(QString, QString)", data ) )
	KMessageBox::error(0, i18n("Unable to add a new Note!"));
    
    // reinit knotes and refetch notes
    initKNotes();
}


#include "knotes_part.moc"
