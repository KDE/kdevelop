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
#include "cppsupportpart.h"
#include "kdevpartcontroller.h"
#include "kdevmainwindow.h"
#include "configproblemreporter.h"
#include "backgroundparser.h"

#include <kdeversion.h>
#include <kparts/part.h>
#include <ktexteditor/editinterface.h>
#include <ktexteditor/document.h>
#include <ktexteditor/markinterface.h>

#if (KDE_VERSION > 305)
# include <ktexteditor/markinterfaceextension.h>
#else
# include "kde30x_markinterfaceextension.h"
#endif

#include <kdebug.h>
#include <klocale.h>
#include <kstatusbar.h>
#include <kurl.h>
#include <kapplication.h>
#include <kiconloader.h>

#include <kconfig.h>
#include <kdebug.h>

#include <qtimer.h>
#include <qregexp.h>
#include <qvbox.h>
#include <qfileinfo.h>
#include <kdialogbase.h>


class ProblemItem: public KListViewItem{
public:
    ProblemItem( QListView* parent, const QString& level, const QString& problem,
		 const QString& file, const QString& line, const QString& column  )
	: KListViewItem( parent, level, problem, file, line, column ) {}
    
    ProblemItem( QListViewItem* parent, const QString& level, const QString& problem,
		 const QString& file, const QString& line, const QString& column  )
	: KListViewItem( parent, level, problem, file, line, column ) {}
    
    int compare( QListViewItem* item, int column, bool ascending ) const {
	if( column == 2 || column == 3 ){
	    int a = text( column ).toInt();
	    int b = item->text( column ).toInt();
	    if( a == b )
		return 0;
	    return( a > b ? 1 : -1 );
	}
	return KListViewItem::compare( item, column, ascending );
    }
    
};

ProblemReporter::ProblemReporter( CppSupportPart* part, QWidget* parent, const char* name )
    : KListView( parent, name ),
      m_cppSupport( part ),
      m_document( 0 ),
      m_markIface( 0 )
{
    addColumn( i18n("Level") );
    addColumn( i18n("File") );
    addColumn( i18n("Line") );
    addColumn( i18n("Column") );
    addColumn( i18n("Problem") );
    setAllColumnsShowFocus( TRUE );

    m_timer = new QTimer( this );

    connect( part->partController(), SIGNAL(activePartChanged(KParts::Part*)),
             this, SLOT(slotActivePartChanged(KParts::Part*)) );
    connect( part->partController(), SIGNAL(partAdded(KParts::Part*)),
             this, SLOT(slotPartAdded(KParts::Part*)) );
    connect( part->partController(), SIGNAL(partRemoved(KParts::Part*)),
             this, SLOT(slotPartRemoved(KParts::Part*)) );

    connect( m_timer, SIGNAL(timeout()), this, SLOT(reparse()) );

    connect( this, SIGNAL(executed(QListViewItem*)),
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
    
    if( m_document ){
	reparse();
	disconnect( m_document, 0, this, 0 );
    }
    
    m_document = dynamic_cast<KTextEditor::Document*>( part );
    
    if( m_document ) {
	m_filename = m_document->url().path();
		
	if( m_cppSupport->fileExtensions().contains(QFileInfo(m_filename).extension()) ){
	    
	    connect( m_document, SIGNAL(textChanged()), this, SLOT(slotTextChanged()) );	    
	    m_markIface = dynamic_cast<KTextEditor::MarkInterface*>( part );	    
	    m_timer->changeInterval( m_delay );
	}
    }
}

void ProblemReporter::slotTextChanged()
{
    if( m_active )
        m_timer->changeInterval( m_delay );
}

void ProblemReporter::removeAllErrors( const QString& filename )
{
    QListViewItem* current = firstChild();
    while( current ){
	QListViewItem* i = current;
	current = current->nextSibling();

	if( i->text(1) == filename )
	    delete( i );
    }

    if( m_markIface ){
	QPtrList<KTextEditor::Mark> marks = m_markIface->marks();
	QPtrListIterator<KTextEditor::Mark> it( marks );
	while( it.current() ){
	    m_markIface->removeMark( it.current()->line, KTextEditor::MarkInterface::markType10 );
	    ++it;
	}
    }
}

void ProblemReporter::reparse()
{
    kdDebug(9007) << "ProblemReporter::reparse()" << endl;
 
    m_timer->stop();
    m_cppSupport->backgroundParser()->reparse();        
}

void ProblemReporter::slotSelected( QListViewItem* item )
{
    KURL url( item->text(1) );
    int line = item->text( 2 ).toInt();
    // int column = item->text( 3 ).toInt();
    m_cppSupport->partController()->editDocument( url, line-1 );
}

void ProblemReporter::reportError( QString message,
				   QString filename,
				   int line, int column )
{
    if( m_markIface && m_filename == filename ){
	m_markIface->addMark( line, KTextEditor::MarkInterface::markType10 );
    }	
    
    new ProblemItem( this,
		     "error",
		     filename,
		     QString::number( line+1 ),
		     QString::number( column+1 ),
		     message.replace( QRegExp("\n"), "" ) );
}

void ProblemReporter::reportWarning( QString message,
                                     QString filename,
                                     int line, int column )
{
    new ProblemItem( this,
		     "warning",
		     filename,
		     QString::number( line+1 ),
		     QString::number( column+1 ),
		     message.replace( QRegExp("\n"), "" ) );
}

void ProblemReporter::reportMessage( QString message,
                                     QString filename,
                                     int line, int column )
{
    new ProblemItem( this,
		     "message",
		     filename,
		     QString::number( line+1 ),
		     QString::number( column+1 ),
		     message.replace( QRegExp("\n"), "" ) );
}

void ProblemReporter::configure()
{
    kdDebug(9007) << "ProblemReporter::configure()" << endl;
    KConfig* config = kapp->config();
    config->setGroup( "General Options" );
    m_active = config->readBoolEntry( "EnableCppBgParser", TRUE );
    m_delay = config->readNumEntry( "CppBgParserDelay", 250 );
}

void ProblemReporter::configWidget( KDialogBase* dlg )
{
    QVBox *vbox = dlg->addVBoxPage(i18n("C++ Parsing"));
    ConfigureProblemReporter* w = new ConfigureProblemReporter( vbox );
    connect(dlg, SIGNAL(okClicked()), w, SLOT(accept()));
    connect(dlg, SIGNAL(okClicked()), this, SLOT(configure()));
}

void ProblemReporter::slotPartAdded( KParts::Part* part )
{
    KTextEditor::MarkInterfaceExtension* iface = dynamic_cast<KTextEditor::MarkInterfaceExtension*>( part );
    
    if( !iface )
	return;
    
    iface->setPixmap( KTextEditor::MarkInterface::markType10, SmallIcon("stop") );
}

void ProblemReporter::slotPartRemoved( KParts::Part* part )
{
    kdDebug(9007) << "ProblemReporter::slotPartRemoved()" << endl;
    if( part == m_document ){
	m_document = 0;
	m_timer->stop();
    }
}

#include "problemreporter.moc"
