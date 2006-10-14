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

#ifndef BACKGROUNDPARSER_H
#define BACKGROUNDPARSER_H

#include "driver.h"
#include "ast.h"

#include <qthread.h>
#include <qwaitcondition.h>
#include <qmutex.h>
#include <qmap.h>
#include <kdebug.h>

class CppSupportPart;
class TranslationUnitAST;
class SynchronizedFileList;

class Unit
{
public:
	Unit() : translationUnit( 0 )
	{}
	~Unit()
	{
		translationUnit = 0;
	}

	QString fileName;
	QValueList<Problem> problems;
	ParsedFilePointer translationUnit;

protected:
	Unit( const Unit& source );
	void operator = ( const Unit& source );
};

class BackgroundParser: public QThread
{
public:
	BackgroundParser( CppSupportPart*, QWaitCondition* consumed );
	virtual ~BackgroundParser();

	QMutex& mutex()
	{
		return m_mutex;
	}
	void lock ()
	{
		m_mutex.lock();
	}
	void unlock()
	{
		m_mutex.unlock();
	}

	QWaitCondition& canParse()
	{
		return m_canParse;
	}
	QWaitCondition& isEmpty()
	{
		return m_isEmpty;
	}

	bool filesInQueue();

    void addFile( const QString& fileName, bool readFromDisk = false );
    void addFileFront( const QString& fileName, bool readFromDisk = false );
    void removeFile( const QString& fileName );
	void removeAllFiles();

    bool hasTranslationUnit( const QString& fileName );
	ParsedFilePointer translationUnit( const QString& fileName );
	QValueList<Problem> problems( const QString& fileName , bool readFromDisk = false, bool forceParse = false );
	void updateParserConfiguration();

	void close();

	virtual void run();

protected:
	Unit* findUnit( const QString& fileName );
	Unit* parseFile( const QString& fileName, bool readFromDisk, bool lock = false )
		;

private:
	class KDevDriver* m_driver;
	QString m_currentFile;
	QWaitCondition m_canParse;
	QWaitCondition m_isEmpty;
	QWaitCondition* m_consumed;
	QMutex m_mutex;
	SynchronizedFileList* m_fileList;
	CppSupportPart* m_cppSupport;
	bool m_close;
	QMap<QString, Unit*> m_unitDict;
};

#endif 
// kate: indent-mode csands; tab-width 4;

