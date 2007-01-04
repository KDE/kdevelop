/*
   Copyright (C) 2002 by Roberto Raggi <roberto@kdevelop.org>

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
#include "pascalsupport_part.h"
#include "kdevpartcontroller.h"
#include "kdevmainwindow.h"
#include "configproblemreporter.h"
#include "backgroundparser.h"

#include <kdeversion.h>
#include <kparts/part.h>
#include <ktexteditor/editinterface.h>
#include <ktexteditor/document.h>
#include <ktexteditor/markinterface.h>

#include <ktexteditor/markinterfaceextension.h>

#include <kdebug.h>
#include <klocale.h>
#include <kstatusbar.h>
#include <kurl.h>
#include <kapplication.h>
#include <kiconloader.h>

#include <kconfig.h>

#include <qtimer.h>
#include <qregexp.h>
#include <qvbox.h>
#include <kdialogbase.h>


class ProblemItem: public QListViewItem{
public:
	ProblemItem( QListView* parent, const QString& level, const QString& problem,
				 const QString& file, const QString& line, const QString& column  )
		: QListViewItem( parent, level, problem, file, line, column ) {}

	ProblemItem( QListViewItem* parent, const QString& level, const QString& problem,
				 const QString& file, const QString& line, const QString& column  )
		: QListViewItem( parent, level, problem, file, line, column ) {}

	int compare( QListViewItem* item, int column, bool ascending ) const {
		if( column == 3 || column == 4 ){
			int a = text( column ).toInt();
			int b = item->text( column ).toInt();
			if( a == b )
				return 0;
			return( a > b ? -1 : 1 );
		}
		return QListViewItem::compare( item, column, ascending );
	}

};

ProblemReporter::ProblemReporter( PascalSupportPart* part, QWidget* parent, const char* name )
    : QListView( parent, name ),
      m_pascalSupport( part ),
      m_editor( 0 ),
      m_document( 0 ),
	  m_markIface( 0 ),
      m_bgParser( 0 )
{
    addColumn( i18n("Level") );
    addColumn( i18n("Problem") );
    addColumn( i18n("File") );
    addColumn( i18n("Line") );
    //addColumn( i18n("Column") );
    setAllColumnsShowFocus( TRUE );

    m_timer = new QTimer( this );

    connect( part->partController(), SIGNAL(activePartChanged(KParts::Part*)),
             this, SLOT(slotActivePartChanged(KParts::Part*)) );
    connect( part->partController(), SIGNAL(partAdded(KParts::Part*)),
             this, SLOT(slotPartAdded(KParts::Part*)) );
    connect( part->partController(), SIGNAL(partRemoved(KParts::Part*)),
             this, SLOT(slotPartRemoved(KParts::Part*)) );

    connect( m_timer, SIGNAL(timeout()), this, SLOT(reparse()) );

    connect( this, SIGNAL(doubleClicked(QListViewItem*)),
             this, SLOT(slotSelected(QListViewItem*)) );
    connect( this, SIGNAL(returnPressed(QListViewItem*)),
             this, SLOT(slotSelected(QListViewItem*)) );

    configure();
}

ProblemReporter::~ProblemReporter()
{
    if( m_bgParser ) {
        m_bgParser->wait();
    }

    delete( m_bgParser );
    m_bgParser = 0;
}

void ProblemReporter::slotActivePartChanged( KParts::Part* part )
{
    if( !part )
        return;
    
    if( m_editor )
	reparse();
		
    m_document = dynamic_cast<KTextEditor::Document*>( part );
    if( m_document ){
        m_filename = m_document->url().path();
    }

    m_editor = dynamic_cast<KTextEditor::EditInterface*>( part );
    if( m_editor )
        connect( m_document, SIGNAL(textChanged()), this, SLOT(slotTextChanged()) );

	m_markIface = dynamic_cast<KTextEditor::MarkInterface*>( part );

	m_timer->changeInterval( m_delay );
}

void ProblemReporter::slotTextChanged()
{
    if( m_active )
        m_timer->changeInterval( m_delay );
}

void ProblemReporter::reparse()
{
    kdDebug(9007) << "ProblemReporter::reparse()" << endl;

    kdDebug() << "1" << endl;

    if( !m_editor )
        return;

    kdDebug() << "2" << endl;

    m_timer->stop();

    kdDebug() << "3" << endl;

    if( m_bgParser ) {
        if( m_bgParser->running() ) {
            m_timer->changeInterval( m_delay );
            return;
        }

        delete( m_bgParser );
        m_bgParser = 0;
    }

    kdDebug() << "4" << endl;

    QListViewItem* current = firstChild();
    while( current ){
        QListViewItem* i = current;
        current = current->nextSibling();

        if( i->text(2) == m_filename )
            delete( i );
    }

    kdDebug() << "5" << endl;

	if( m_markIface ){
		QPtrList<KTextEditor::Mark> marks = m_markIface->marks();
		QPtrListIterator<KTextEditor::Mark> it( marks );
		while( it.current() ){
			m_markIface->removeMark( it.current()->line, KTextEditor::MarkInterface::markType07 );
			++it;
		}
	}

    kdDebug() << "6" << endl;

    m_bgParser = new BackgroundParser( this, m_editor->text(), m_filename );

    kdDebug() << "7" << endl;

    m_bgParser->start();

    kdDebug() << "8" << endl;
}

void ProblemReporter::slotSelected( QListViewItem* item )
{
    KURL url( item->text(2) );
    int line = item->text( 3 ).toInt();
    // int column = item->text( 4 ).toInt();
    m_pascalSupport->partController()->editDocument( url, line-1 );
}

void ProblemReporter::reportError( QString message,
                                   QString filename,
                                   int line, int column )
{
	if( m_markIface ){
		m_markIface->addMark( line-1, KTextEditor::MarkInterface::markType07 );
	}
	
    new ProblemItem( this,
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
    new ProblemItem( this,
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
    config->setGroup( "General Options" );
    m_active = config->readBoolEntry( "EnableCppBgParser", TRUE );
    m_delay = config->readNumEntry( "BgParserDelay", 250 );
}

void ProblemReporter::configWidget( KDialogBase* dlg )
{
    /// \FIXME adymo: use problem reporter configuration
    Q_UNUSED(dlg);
/*    QVBox *vbox = dlg->addVBoxPage(i18n("Pascal Parsing"));
    ConfigureProblemReporter* w = new ConfigureProblemReporter( vbox );
    connect(dlg, SIGNAL(okClicked()), w, SLOT(accept()));
    connect(dlg, SIGNAL(okClicked()), this, SLOT(configure()));*/
}

void ProblemReporter::slotPartAdded( KParts::Part* part )
{
	KTextEditor::MarkInterfaceExtension* iface = dynamic_cast<KTextEditor::MarkInterfaceExtension*>( part );
	
	if( !iface )
		return;
		
	iface->setPixmap( KTextEditor::MarkInterface::markType07, SmallIcon("stop") );
}

void ProblemReporter::slotPartRemoved( KParts::Part* part )
{
    kdDebug(9007) << "ProblemReporter::slotPartRemoved()" << endl;
    if( part == m_document ){
        m_document = 0;
        m_editor = 0;
        m_timer->stop();
    }
}

#include "problemreporter.moc"
