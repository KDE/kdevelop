/*
   Copyright (C) 2002 by Roberto Raggi <raggi@cli.di.unipi.it>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   version 2, License as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "problemreporter.h"
#include "javasupportpart.h"
#include "kdevpartcontroller.h"
#include "kdevtoplevel.h"

#include <kparts/part.h>
#include <ktexteditor/editinterface.h>
#include <ktexteditor/document.h>
#include <kdebug.h>
#include <klocale.h>
#include <kstatusbar.h>
#include <kurl.h>

#include <qtimer.h>
#include <qregexp.h>

ProblemReporter::ProblemReporter( JavaSupportPart* part, QWidget* parent, const char* name )
    : QListView( parent, name ), m_javaSupport( part ), m_editor( 0 )
{
    addColumn( i18n("Level") );
    addColumn( i18n("Problem") );
    addColumn( i18n("File") );
    addColumn( i18n("Line") );
    addColumn( i18n("Column") );
    setAllColumnsShowFocus( TRUE );

    m_timer = new QTimer( this );

    connect( part->partController(), SIGNAL(activePartChanged(KParts::Part*)),
             this, SLOT(slotActivePartChanged(KParts::Part*)) );

    connect( m_timer, SIGNAL(timeout()), this, SLOT(reparse()) );

    connect( this, SIGNAL(doubleClicked(QListViewItem*)),
             this, SLOT(slotSelected(QListViewItem*)) );
    connect( this, SIGNAL(returnPressed(QListViewItem*)),
             this, SLOT(slotSelected(QListViewItem*)) );
}

ProblemReporter::~ProblemReporter()
{

}

void ProblemReporter::slotActivePartChanged( KParts::Part* part )
{
    if( !part )
        return;

    m_editor = dynamic_cast<KTextEditor::EditInterface*>( part );
    if( m_editor ){
        connect( part, SIGNAL(textChanged()), this, SLOT(slotTextChanged()) );
    }

    KTextEditor::Document* doc = dynamic_cast<KTextEditor::Document*>( part );
    if( doc ){
        m_filename = doc->url().path();
    }
}

void ProblemReporter::slotTextChanged()
{
    m_timer->changeInterval( 500 );
}

void ProblemReporter::reparse()
{
    if( !m_editor )
        return;

    QListViewItem* current = firstChild();
    while( current ){
        QListViewItem* i = current;
        current = current->nextSibling();

        if( i->text(2) == m_filename )
            delete( i );
    }

    m_javaSupport->parseContents( m_editor->text(), m_filename );
    m_timer->stop();
}

void ProblemReporter::slotSelected( QListViewItem* item )
{
    KURL url( item->text(2) );
    int line = item->text( 3 ).toInt();
    // int column = item->text( 4 ).toInt();
    m_javaSupport->partController()->editDocument( url, line-1 );
}

void ProblemReporter::reportError( QString message,
                                   QString filename,
                                   int line, int column )
{
    new QListViewItem( this,
                       "error",
                       message.replace( QRegExp("\n"), "" ),
                       filename,
                       QString::number( line ),
                       QString::number( column ) );
}

void ProblemReporter::reportWarning( QString message,
                                     QString filename,
                                     int line, int column )
{
    new QListViewItem( this,
                       "warning",
                       message.replace( QRegExp("\n"), "" ),
                       filename,
                       QString::number( line ),
                       QString::number( column ) );
}

void ProblemReporter::reportMessage( QString message,
                                     QString filename,
                                     int line, int column )
{
    new QListViewItem( this,
                       "message",
                       message.replace( QRegExp("\n"), "" ),
                       filename,
                       QString::number( line ),
                       QString::number( column ) );
}
