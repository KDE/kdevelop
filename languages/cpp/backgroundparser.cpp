/***************************************************************************
*   Copyright (C) 2002 by Roberto Raggi                                   *
*   roberto@kdevelop.org                                                 *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "backgroundparser.h"
#include "cppsupportpart.h"
#include "cppsupport_events.h"
#include "codeinformationrepository.h"
#include "cppcodecompletion.h"
#include "driver.h"
#include "ast_utils.h"
#include "kdevdeepcopy.h"
#include "kdevdriver.h"

#include <qmutex.h>

#include <kparts/part.h>
#include <ktexteditor/editinterface.h>
#include <ktexteditor/document.h>
#include <ktexteditor/view.h>

#include <kdevpartcontroller.h>
#include <kdevproject.h>

#include <kurl.h>
#include <kdebug.h>
#include <kapplication.h>

#include <qfile.h>
#include <qfileinfo.h>
#include <qtextstream.h>
#include <qprocess.h>
#include <list>


class KDevSourceProvider: public SourceProvider
{
public:
	KDevSourceProvider( CppSupportPart* cppSupport )
		: m_cppSupport( cppSupport ),
		m_readFromDisk( false )
	{}
	
	void setReadFromDisk( bool b )
	{
		m_readFromDisk = b;
	}
	bool readFromDisk() const
	{
		return m_readFromDisk;
	}
	
	virtual QString contents( const QString& fileName )
	{
		QString contents = QString::null;
		
		if ( !m_readFromDisk )
		{
			// GET LOCK
			kapp->lock ()
				;
			
			//kdDebug(9007) << "-------> kapp locked" << endl;
			
			QPtrList<KParts::Part> parts( *m_cppSupport->partController() ->parts() );
			QPtrListIterator<KParts::Part> it( parts );
			while ( it.current() )
			{
				KTextEditor::Document * doc = dynamic_cast<KTextEditor::Document*>( it.current() );
				++it;
				
				KTextEditor::EditInterface* editIface = dynamic_cast<KTextEditor::EditInterface*>( doc );
				if ( !doc || !editIface || doc->url().path() != fileName )
					continue;
				
				contents = QString( editIface->text().ascii() ); // deep copy
				
				//kdDebug(9007) << "-------> kapp unlocked" << endl;
				
				break;
			}
			
			// RELEASE LOCK
			kapp->unlock();
			//kdDebug(9007) << "-------> kapp unlocked" << endl;
		}
		
		if( m_readFromDisk || contents == QString::null )
		{
			QFile f( fileName );
			if ( f.open( IO_ReadOnly ) )
			{
				QTextStream stream( &f );
				contents = stream.read();
				f.close();
			}
		}
		
		return contents;
	}
	
	virtual bool isModified( const QString& fileName )
	{
		Q_UNUSED( fileName );
		return true;
	}
	
private:
	CppSupportPart* m_cppSupport;
	bool m_readFromDisk;
private:
	KDevSourceProvider( const KDevSourceProvider& source );
	void operator = ( const KDevSourceProvider& source );
};

typedef std::string SafeString;

class SynchronizedFileList
{
  typedef std::list< QPair<SafeString, bool> > ListType;
public:
	SynchronizedFileList()
	{}
	
	bool isEmpty() const
	{
		QMutexLocker locker( &m_mutex );
		return m_fileList.empty();
	}
	
	uint count() const
	{
		QMutexLocker locker( &m_mutex );
		return m_fileList.size();
	}
	
	QPair<SafeString, bool> front() const
	{
		QMutexLocker locker( &m_mutex );
		return m_fileList.front();
	}
	
	void clear()
	{
		QMutexLocker locker( &m_mutex );
		m_fileList.clear();
	}
	
	void push_front( const QString& fileName, bool readFromDisk = false )
	{
		SafeString s( fileName.ascii() );
		QMutexLocker locker( &m_mutex );
		m_fileList.push_front( qMakePair( s, readFromDisk ) );
	}

	void push_back( const QString& fileName, bool readFromDisk = false )
	{
	        SafeString s( fileName.ascii() );
		QMutexLocker locker( &m_mutex );
		m_fileList.push_back( qMakePair( s, readFromDisk ) );
	}
	
	void pop_front()
	{
		QMutexLocker locker( &m_mutex );
		m_fileList.pop_front();
	}

  QPair<SafeString, bool> takeFront()
  {
    QMutexLocker locker( &m_mutex );
    QPair<SafeString, bool> ret = m_fileList.front();
    m_fileList.pop_front();
    return ret;
  }
	
	bool contains( const QString& fileName ) const
	{
		QMutexLocker locker( &m_mutex );
		ListType::const_iterator it = m_fileList.begin();
		while ( it != m_fileList.end() )
		{
			if ( ( *it ).first.compare( fileName.ascii() ) == 0 )
				return true;
			++it;
		}
		return false;
	}
	
	void remove( const QString& fileName )
	{
		QMutexLocker locker( &m_mutex );
		ListType::iterator it = m_fileList.begin();
		while ( it != m_fileList.end() )
		{
			if ( ( *it ).first.compare(fileName.ascii() ) == 0 )
				m_fileList.erase( it++ );
			else
			  ++it;
		}
	}
	
private:
	mutable QMutex m_mutex;
	ListType m_fileList;
};

BackgroundParser::BackgroundParser( CppSupportPart* part, QWaitCondition* consumed )
: m_consumed( consumed ), m_cppSupport( part ), m_close( false )
{
	m_fileList = new SynchronizedFileList();
	m_driver = new KDevDriver( m_cppSupport );
	m_driver->setSourceProvider( new KDevSourceProvider( m_cppSupport ) );
	
	QString conf_file_name = m_cppSupport->specialHeaderName();
	if ( QFile::exists( conf_file_name ) )
		m_driver->parseFile( conf_file_name, true );
	
	//disabled for now m_driver->setResolveDependencesEnabled( true );
}

BackgroundParser::~BackgroundParser()
{
	removeAllFiles();
	
	delete( m_driver );
	m_driver = 0;
	
	delete m_fileList;
	m_fileList = 0;
}

void BackgroundParser::addFile( const QString& fileName, bool readFromDisk )
{
	QString fn = deepCopy( fileName );
	
	bool added = false;
	/*if ( !m_fileList->contains( fn ) )
	{
		m_fileList->push_back( fn, readFromDisk );
		added = true;
	}*/
	
	if ( added )
		m_canParse.wakeAll();
}

void BackgroundParser::addFileFront( const QString& fileName, bool readFromDisk )
{
	QString fn = deepCopy( fileName );
	
	bool added = false;
	/*if ( m_fileList->contains( fn ) )
		m_fileList->remove( fn );*/

	m_fileList->push_front( fn, readFromDisk );
	added = true;
	
	if ( added )
		m_canParse.wakeAll();
}

void BackgroundParser::removeAllFiles()
{
	kdDebug( 9007 ) << "BackgroundParser::removeAllFiles()" << endl;
	QMutexLocker locker( &m_mutex );
	
	QMap<QString, Unit*>::Iterator it = m_unitDict.begin();
	while ( it != m_unitDict.end() )
	{
		Unit * unit = it.data();
		++it;
		delete( unit );
		unit = 0;
	}
	m_unitDict.clear();
	m_driver->reset();
	m_fileList->clear();
	
	m_isEmpty.wakeAll();
}

void BackgroundParser::removeFile( const QString& fileName )
{
	QMutexLocker locker( &m_mutex );
	
	Unit* unit = findUnit( fileName );
	if ( unit )
	{
		m_driver->remove
			( fileName );
		m_unitDict.remove( fileName );
		delete( unit );
		unit = 0;
	}
	
	if ( m_fileList->isEmpty() )
		m_isEmpty.wakeAll();
}

Unit* BackgroundParser::parseFile( const QString& fileName, bool readFromDisk, bool lock )
{
	static_cast<KDevSourceProvider*>( m_driver->sourceProvider() ) ->setReadFromDisk( readFromDisk );
	
	m_driver->remove( fileName );
	m_driver->parseFile( fileName , false, true );
    if( !m_driver->isResolveDependencesEnabled() )
        m_driver->removeAllMacrosInFile( fileName );  // romove all macros defined by this
	// translation unit.
	ParsedFilePointer translationUnit = m_driver->takeTranslationUnit( fileName );
	
	Unit* unit = new Unit;
	unit->fileName = fileName;
	unit->translationUnit = translationUnit;
	unit->problems = m_driver->problems( fileName );
	
	static_cast<KDevSourceProvider*>( m_driver->sourceProvider() ) ->setReadFromDisk( false );
	
	if ( lock )
		m_mutex.lock();
	
	if ( m_unitDict.find( fileName ) != m_unitDict.end() )
	{
		Unit * u = m_unitDict[ fileName ];
		m_unitDict.remove( fileName );
		delete( u );
		u = 0;
	}
	
	m_unitDict.insert( fileName, unit );
	
	if ( lock )
		m_mutex.unlock();
	
	KApplication::postEvent( m_cppSupport, new FileParsedEvent( fileName, unit->problems, readFromDisk ) );
	
	m_currentFile = QString::null;
	
	if ( m_fileList->isEmpty() )
		m_isEmpty.wakeAll();
	
	return unit;
}

Unit* BackgroundParser::findUnit( const QString& fileName )
{
	QMap<QString, Unit*>::Iterator it = m_unitDict.find( fileName );
	return it != m_unitDict.end() ? *it : 0;
}

bool BackgroundParser::hasTranslationUnit( const QString& fileName ) {
	QMap<QString, Unit*>::Iterator it = m_unitDict.find( fileName );
	return it != m_unitDict.end();
}

ParsedFilePointer BackgroundParser::translationUnit( const QString& fileName )
{
	Unit * u = findUnit( fileName );
	if ( u == 0 )
	{
        return 0;
        /*m_fileList->remove
			( fileName );
        u = parseFile( fileName, false );*/
	}
	
	return u->translationUnit;
}

QValueList<Problem> BackgroundParser::problems( const QString& fileName, bool readFromDisk, bool forceParse )
{
	Unit * u = findUnit( fileName );
	if ( u == 0 || forceParse )
	{
        /*   
		m_fileList->remove
			( fileName );
        u = parseFile( fileName, readFromDisk ); */
	}
	
	return u ? u->problems : QValueList<Problem>();
}

void BackgroundParser::close()
{
  {
	QMutexLocker locker( &m_mutex );
	m_close = true;
	m_canParse.wakeAll();
  }
	kapp->unlock();
	
	while ( running() )
		sleep( 1 );
	kapp->lock();
}

bool BackgroundParser::filesInQueue()
{
	QMutexLocker locker( &m_mutex );
	
	return m_fileList->count() || !m_currentFile.isEmpty();
}

void BackgroundParser::updateParserConfiguration()
{
	QMutexLocker locker( &m_mutex );
	
	m_driver->setup();
	QString conf_file_name = m_cppSupport->specialHeaderName();
	m_driver->removeAllMacrosInFile( conf_file_name );
	m_driver->parseFile( conf_file_name, true, true );
}

void BackgroundParser::run()
{
	// (void) m_cppSupport->codeCompletion()->repository()->getEntriesInScope( QStringList(), false );
	
	while ( !m_close )
	{
		
		while ( m_fileList->isEmpty() )
		{
			m_canParse.wait();
			
			if ( m_close )
				break;
		}
		
		if ( m_close )
			break;
		
		QPair<SafeString, bool> entry = m_fileList->takeFront();
		QString fileName = entry.first.c_str();
		bool readFromDisk = entry.second;
		m_currentFile = fileName;
		
		( void ) parseFile( fileName, readFromDisk, true );

		
		m_currentFile = QString::null;
	}
	
	kdDebug( 9007 ) << "!!!!!!!!!!!!!!!!!! BG PARSER DESTROYED !!!!!!!!!!!!" << endl;
	
//	adymo: commented to fix #88091
//	QThread::exit();
}

//kate: indent-mode csands; tab-width 4; space-indent off;
