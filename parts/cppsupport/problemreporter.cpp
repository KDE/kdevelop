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
#include "cppsupportpart.h"
#include "kdevpartcontroller.h"
#include "kdevtoplevel.h"

#include <kparts/part.h>
#include <ktexteditor/editinterface.h>
#include <ktexteditor/document.h>
#include <kdebug.h>
#include <klocale.h>
#include <kstatusbar.h>
#include <kurl.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kdebug.h>

#include <qtimer.h>
#include <qregexp.h>


ProblemReporter::ProblemReporter( CppSupportPart* part, QWidget* parent, const char* name )
    : QListView( parent, name ), m_cppSupport( part ), m_editor( 0 ), m_document( 0 )
{
    addColumn( i18n("Level") );
    addColumn( i18n("Problem") );
    addColumn( i18n("File") );
    addColumn( i18n("Line") );
//    addColumn( i18n("Column") );
    setAllColumnsShowFocus( TRUE );

    m_timer = new QTimer( this );

    connect( part->partController(), SIGNAL(activePartChanged(KParts::Part*)),
             this, SLOT(slotActivePartChanged(KParts::Part*)) );

    connect( m_timer, SIGNAL(timeout()), this, SLOT(reparse()) );

    connect( this, SIGNAL(doubleClicked(QListViewItem*)),
             this, SLOT(slotSelected(QListViewItem*)) );
    connect( this, SIGNAL(returnPressed(QListViewItem*)),
             this, SLOT(slotSelected(QListViewItem*)) );

    configure();
}

ProblemReporter::~ProblemReporter()
{

}

void ProblemReporter::slotActivePartChanged( KParts::Part* part )
{
    if( !part )
        return;

    m_document = dynamic_cast<KTextEditor::Document*>( part );
    if( m_document ){
        m_filename = m_document->url().path();
    }

    m_editor = dynamic_cast<KTextEditor::EditInterface*>( part );
    if( m_editor )
        connect( m_document, SIGNAL(textChanged()), this, SLOT(slotTextChanged()) );
}

void ProblemReporter::slotTextChanged()
{
    if( m_active )
        m_timer->changeInterval( m_delay );
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

    m_cppSupport->parseContents( m_editor->text(), m_filename );
    m_timer->stop();
}

void ProblemReporter::slotSelected( QListViewItem* item )
{
    KURL url( item->text(2) );
    int line = item->text( 3 ).toInt();
    // int column = item->text( 4 ).toInt();
    m_cppSupport->partController()->editDocument( url, line-1 );
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

void ProblemReporter::configure()
{
    kdDebug(9007) << "ProblemReporter::configure()" << endl;
    KConfig* config = kapp->config();
    m_active = config->readBoolEntry( "EnableCppBgParser", TRUE );
    m_delay = config->readNumEntry( "CppBgParserDelay", 1000 );
}

#include "problemreporter.moc"
