/***************************************************************************
*   Copyright (C) 2003 by Roberto Raggi                                   *
*   roberto@kdevelop.org                                                  *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "createpcsdialog.h"
#include "createpcsdialog.moc"
#include "driver.h"
#include "tag_creator.h"
#include "cppsupportpart.h"

#include <catalog.h>
#include <kdevpcsimporter.h>
#include <kdevcoderepository.h>

#include <kparts/componentfactory.h>

#include <ktrader.h>
#include <kdebug.h>
#include <klibloader.h>
#include <klistbox.h>
#include <kiconloader.h>
#include <klistview.h>
#include <kapplication.h>
#include <kinstance.h>
#include <kstandarddirs.h>

#include <qprogressbar.h>
#include <qheader.h>
#include <qlabel.h>
#include <qprocess.h>
#include <qregexp.h>
#include <qlayout.h>

#include <stdlib.h>
#include <unistd.h>

class CreatePCSDialog::RppDriver: public Driver
{
public:
	RppDriver( Catalog* c )
			: catalog( c )
	{
		setup();
	}

	virtual ~RppDriver()
	{
#if 0 /// \FIXME robe
		TagCreator::destroyDocumentation();
#endif

	}

#if 0 /// \FIXME robe
	void addDocDirectory( const QString& dir )
	{
		m_docDirectoryList.append( dir );
		TagCreator::setDocumentationDirectories( m_docDirectoryList );
	}
#endif

	void fileParsed( const QString& fileName )
	{
		/// @todo increment progress

#if 0 /// @todo show problems
		QValueList<Problem> l = problems( fileName );
		QValueList<Problem>::Iterator it = l.begin();
		while ( it != l.end() )
		{
			const Problem & p = *it;
			++it;
		}
#endif

		TranslationUnitAST::Node ast = takeTranslationUnit( fileName );

		TagCreator w( fileName, catalog );
		w.parseTranslationUnit( ast.get() );

		//if( !isResolveDependencesEnabled() )
		//    removeAllMacrosInFile( fileName );
	}

	// setup the preprocessor
	// code provided by Reginald Stadlbauer <reggie@trolltech.com>
	void setup()
	{
		QProcess proc;
		proc.addArgument( "gcc" );
		proc.addArgument( "-print-file-name=include" );
		if ( !proc.start() )
		{
			/// @todo message box
			return ;
		}

		while ( proc.isRunning() )
			usleep( 1 );

		QString gccLibPath = proc.readStdout();
		gccLibPath = gccLibPath.replace( QRegExp( "[\r\n]" ), "" );
		addIncludePath( gccLibPath );
		//addIncludePath( "/usr/include/g++-3" );
		//addIncludePath( "/usr/include/g++" );
		proc.clearArguments();
		proc.addArgument( "gcc" );
		proc.addArgument( "-E" );
		proc.addArgument( "-dM" );
		proc.addArgument( "-ansi" );
		proc.addArgument( "-" );
		
		if ( !proc.start() )
		{
			/// @todo message box
			return ;
		}
		
		while ( !proc.isRunning() )
			usleep( 1 );
		proc.closeStdin();
		while ( proc.isRunning() )
			usleep( 1 );
		
		while ( proc.canReadLineStdout() )
		{
			QString l = proc.readLineStdout();
			QStringList lst = QStringList::split( ' ', l );
			if ( lst.count() != 3 )
				continue;
			
			addMacro( Macro( lst[ 1 ], lst[ 2 ] ) );
		}
		addMacro( Macro( "__cplusplus", "1" ) );
	}

private:
	Catalog* catalog;
#if 0 /// \FIXME

	QStringList m_docDirectoryList;
#endif
};


class PCSListViewItem: public KListViewItem
{
public:
	PCSListViewItem( KService::Ptr ptr, KDevPCSImporter* importer, QListViewItem* parent )
			: KListViewItem( parent ), m_importer( importer )
	{
		init( ptr );
	}
	PCSListViewItem( KService::Ptr ptr, KDevPCSImporter* importer, QListView* parent )
			: KListViewItem( parent ), m_importer( importer )
	{
		init( ptr );
	}

	~PCSListViewItem()
	{
		delete( m_importer );
		m_importer = 0;
	}

	void init( KService::Ptr ptr )
	{
		setText( 0, ptr->comment() );
		setPixmap( 0, SmallIcon( ptr->icon() ) );
	}

	KDevPCSImporter* importer()
	{
		return m_importer;
	}

private:
	KDevPCSImporter* m_importer;
};

CreatePCSDialog::CreatePCSDialog( CppSupportPart* part, QWidget* parent, const char* name, bool modal, WFlags fl )
		: CreatePCSDialogBase( parent, name, modal, fl ), m_part( part )
{
	m_settings = 0;
	importerListView->header() ->hide();

	KTrader::OfferList lst = KTrader::self() ->query( "KDevelop/PCSImporter" );
	kdDebug( 9007 ) << "====================> found " << lst.size() << " importers" << endl;

	for ( KTrader::OfferList::Iterator it = lst.begin(); it != lst.end(); ++it )
	{
		KService::Ptr ptr = *it;

		int error = 0;
		KDevPCSImporter* importer = KParts::ComponentFactory::createInstanceFromService<KDevPCSImporter>( ptr, this, ptr->name().latin1(), QStringList(), &error );
		if ( importer )
		{
			new PCSListViewItem( ptr, importer, importerListView );
		}
	}

	setNextEnabled( page, false );

	QHBoxLayout* hbox = new QHBoxLayout( settingsPage );
	hbox->setAutoAdd( true );

	if ( importerListView->firstChild() )
	{
		importerListView->setSelected( importerListView->firstChild(), true );
		setNextEnabled( page, true );
	}
}

CreatePCSDialog::~CreatePCSDialog()
{}

/*$SPECIALIZATION$*/
void CreatePCSDialog::back()
{
	QWizard::back();
}

void CreatePCSDialog::next()
{
	QWizard::next();
}

void CreatePCSDialog::reject()
{
	QWizard::reject();
}

void CreatePCSDialog::accept()
{
	QWizard::accept();
}

void CreatePCSDialog::slotSelected( const QString & )
{
	if ( currentPage() == settingsPage )
	{
		if ( m_settings )
			delete( m_settings );

		KDevPCSImporter* importer = static_cast<PCSListViewItem*>( importerListView->selectedItem() ) ->importer();
		m_settings = importer->createSettingsPage( settingsPage );
		setNextEnabled( currentPage(), false );
		setHelpEnabled( currentPage(), false );
		connect( m_settings, SIGNAL( enabled( int ) ), this, SLOT( setNextPageEnabled( int ) ) );

		if ( m_settings )
		{
			setHelpEnabled( m_settings, false );
			m_settings->show();
		}
	}
	else if ( currentPage() == finalPage )
	{
		setBackEnabled( currentPage(), false );
		setNextEnabled( currentPage(), false );

		KDevPCSImporter* importer = static_cast<PCSListViewItem*>( importerListView->selectedItem() )->importer();
		QStringList fileList = importer->fileList();
		progressBar->setTotalSteps( fileList.size() );
		progressBar->setPercentageVisible( true );
		int n = 0;

		KStandardDirs *dirs = m_part->instance() ->dirs();

		QString dbName = dirs->saveLocation( "data", "kdevcppsupport/pcs" ) + "/" + importer->dbName() + ".db";
		kdDebug( 9007 ) << "================================> dbName = " << dbName << endl;

		m_part->removeCatalog( dbName );

		Catalog* c = new Catalog();
		c->open( dbName );
		c->addIndex( "kind" );
		c->addIndex( "name" );
		c->addIndex( "scope" );
		c->addIndex( "fileName" );

		RppDriver driver( c );
		for ( QStringList::Iterator it = fileList.begin(); it != fileList.end(); ++it )
		{
			//kdDebug(9007) << "==========> parsing: " << *it << endl;
			progressBar->setProgress( ++n );
			currentFile->setText( *it );
			driver.parseFile( *it );
			kapp->processEvents();
		}

		currentFile->setText( "" );
		m_part->addCatalog( c );

		setFinishEnabled( currentPage(), true );
	}
}

void CreatePCSDialog::setNextPageEnabled( int enabled )
{
	setNextEnabled( currentPage(), enabled );
}

void CreatePCSDialog::slotSelectionChanged( QListViewItem * item )
{
	setNextPageEnabled( item != 0 );
}

//kate: indent-mode csands; tab-width 4; space-indent off;
