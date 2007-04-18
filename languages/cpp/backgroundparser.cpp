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
#include <list>
#include <qdatastream.h>


class BackgroundKDevDriver : public KDevDriver {
public:
	BackgroundKDevDriver( CppSupportPart* cppSupport, BackgroundParser* bp ) : KDevDriver( cppSupport, false ), m_backgroundParser(bp) {
	}
	virtual void fileParsed( ParsedFile& fileName );
	virtual void addDependence( const QString& fileName, const Dependence& dep );
private:
	BackgroundParser* m_backgroundParser;
};


class KDevSourceProvider: public SourceProvider
{
public:
	//Deadlock is a mutex that is locked when KDevSourceProvider::contents(..) is used, and that should be unlocked before QApplication is locked(that way a deadlock where the thread that holds the QApplication-mutex and tries to lock the given mutex, while the thread that calls contents(..) and holds the given mutex and tries to lock the QApplication-mutex, cannot happen)
	KDevSourceProvider( CppSupportPart* cppSupport, QMutex& deadlock )
		: m_cppSupport( cppSupport ),
		m_readFromDisk( false ),
		m_deadlock(deadlock)
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
			m_deadlock.unlock();
			// GET LOCK
			kapp->lock ();
			
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
			m_deadlock.lock();
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
		bool ret = false;
		m_deadlock.unlock();
		kapp->lock ();

		KParts::ReadOnlyPart *part = m_cppSupport->partController()->partForURL( KURL(fileName) );
		KTextEditor::Document * doc = dynamic_cast<KTextEditor::Document*>( part );

		if ( doc )
			ret = doc->isModified();

		kapp->unlock();
		m_deadlock.lock();
		return ret;
	}
	
private:
	CppSupportPart* m_cppSupport;
	bool m_readFromDisk;
	QMutex& m_deadlock;
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

	int count( const QString& fileName ) const {
		int c = 0;
	
		QMutexLocker locker( &m_mutex );
		ListType::const_iterator it = m_fileList.begin();
		while ( it != m_fileList.end() )
		{
			if ( ( *it ).first.compare( fileName.ascii() ) == 0 )
				++c;
			++it;
		}
		return c;
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
: m_consumed( consumed ), m_cppSupport( part ), m_close( false ), m_saveMemory( false )
{
	m_fileList = new SynchronizedFileList();
	m_driver = new BackgroundKDevDriver( m_cppSupport, this );
	m_driver->setSourceProvider( new KDevSourceProvider( m_cppSupport,  m_mutex ) );
	
	QString conf_file_name = m_cppSupport->specialHeaderName();
	m_mutex.lock();
	if ( QFile::exists( conf_file_name ) )
		m_driver->parseFile( conf_file_name, true, true, true );
	m_mutex.unlock();
	
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
	
  //bool added = false;
	/*if ( !m_fileList->contains( fn ) )
	{
		m_fileList->push_back( fn, readFromDisk );
		added = true;
	}*/
  m_fileList->push_back( fn, readFromDisk );
  
  //if ( added )
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

void BackgroundKDevDriver::addDependence( const QString& fileName, const Dependence& dep ) {
	//give waiting threads a chance to perform their actions
	m_backgroundParser->m_mutex.unlock();
	m_backgroundParser->m_mutex.lock();
	KDevDriver::addDependence( fileName, dep );
}

void BackgroundKDevDriver::fileParsed( ParsedFile& fileName ) {
	m_backgroundParser->fileParsed( fileName );
}

void BackgroundParser::parseFile( const QString& fileName, bool readFromDisk, bool lock )
{
	if( lock )
		m_mutex.lock();
	m_readFromDisk = readFromDisk;
	static_cast<KDevSourceProvider*>( m_driver->sourceProvider() ) ->setReadFromDisk( readFromDisk );
	
	m_driver->remove( fileName );
	m_driver->parseFile( fileName , false, true );
    if( !m_driver->isResolveDependencesEnabled() )
        m_driver->removeAllMacrosInFile( fileName );  // romove all macros defined by this
	// translation unit.
    if ( lock )
        m_mutex.unlock();
}

QValueList<Problem> cloneProblemList( const QValueList<Problem>& list ) {
	QValueList<Problem> ret;
	for( QValueList<Problem>::const_iterator it = list.begin(); it != list.end(); ++it ) {
		ret << Problem( *it, true );
	}
	return ret;
}

void BackgroundParser::fileParsed( ParsedFile& file ) {

	ParsedFilePointer translationUnitUnsafe = m_driver->takeTranslationUnit( file.fileName() );
	//now file and translationUnitUnsafe are the same
	ParsedFilePointer translationUnit;
	//Since the lexer-cache keeps many QStrings like macro-names used in the background, everything must be copied here. The safest solution is just
	//serializing and deserializing the whole thing(the serialization does not respect the AST, but that can be copied later because that's safe)
	QMemArray<char> data;
	{
	  QDataStream stream( data, IO_WriteOnly );
	  translationUnitUnsafe->write( stream );
	}
	{
	  QDataStream stream( data, IO_ReadOnly );
	  translationUnit = new ParsedFile( stream );
	}

	translationUnit->setTranslationUnit( translationUnitUnsafe->operator TranslationUnitAST *() ); //Copy the AST, doing that is thread-safe
	translationUnitUnsafe->setTranslationUnit( 0 ); //Move the AST completely out of this thread's scope. Else it might crash on dual-core machines
	file.setTranslationUnit(0); //just to be sure, set to zero on both
	
	Unit* unit = new Unit;
	unit->fileName = file.fileName();
	unit->translationUnit = translationUnit;
	unit->problems = cloneProblemList( m_driver->problems( file.fileName() ) );
	
	static_cast<KDevSourceProvider*>( m_driver->sourceProvider() ) ->setReadFromDisk( false );
	
	if ( m_unitDict.find( file.fileName() ) != m_unitDict.end() )
	{
		Unit * u = m_unitDict[ file.fileName() ];
		m_unitDict.remove( file.fileName() );
		delete( u );
		u = 0;
	}
	
	m_unitDict.insert( file.fileName(), unit );
	
	KApplication::postEvent( m_cppSupport, new FileParsedEvent( file.fileName(), unit->problems, m_readFromDisk ) );
	
	m_currentFile = QString::null;
	
  if ( m_fileList->isEmpty() )
		m_isEmpty.wakeAll();	
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
    Q_UNUSED(readFromDisk);
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

int BackgroundParser::countInQueue( const QString& file ) const {
	return m_fileList->count( file );
}

void BackgroundParser::updateParserConfiguration()
{
	QMutexLocker locker( &m_mutex );

	m_driver->setup();
	QString conf_file_name = m_cppSupport->specialHeaderName();
	m_driver->removeAllMacrosInFile( conf_file_name );
	m_driver->parseFile( conf_file_name, true, true, true );
}

void BackgroundParser::run()
{
	// (void) m_cppSupport->codeCompletion()->repository()->getEntriesInScope( QStringList(), false );
	
	while ( !m_close )
	{
		
		while ( m_fileList->isEmpty() )
		{
			if( m_saveMemory ) {
				m_saveMemory = false;
				m_driver->lexerCache()->saveMemory();
			}

			m_canParse.wait();
			
			if ( m_close )
				break;
		}
		
		if ( m_close )
			break;
		
		QPair<SafeString, bool> entry = m_fileList->takeFront();
		QString fileName = entry.first.c_str();
		bool readFromDisk = entry.second;
		m_currentFile = deepCopy(fileName);
		
		( void ) parseFile( fileName, readFromDisk, true );

		
		m_currentFile = QString::null;
	}
	
	kdDebug( 9007 ) << "!!!!!!!!!!!!!!!!!! BG PARSER DESTROYED !!!!!!!!!!!!" << endl;
	
//	adymo: commented to fix #88091
//	QThread::exit();
}

void BackgroundParser::saveMemory() {
	m_saveMemory = true; //Delay the operation
	m_canParse.wakeAll();
}


//kate: indent-mode csands; tab-width 4; space-indent off;
