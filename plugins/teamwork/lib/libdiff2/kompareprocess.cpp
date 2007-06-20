/***************************************************************************
     kompareprocess.cpp  -  description
     -------------------
    begin    : Sun Mar 4 2001
        copyright               : (C) 2001-2003 Otto Bruggeman <otto.bruggeman@home.nl>
        copyright               : (C) 2001-2003 John Firebaugh <jfirebaugh@kde.org>
****************************************************************************/

/***************************************************************************
**
**   This program is free software; you can redistribute it and/or modify
**   it under the terms of the GNU General Public License as published by
**   the Free Software Foundation; either version 2 of the License, or
**   (at your option) any later version.
**
***************************************************************************/

#include "kompareprocess.h"
#include <qdir.h>
#include <qstringlist.h>
#include <qtextcodec.h>
//Added by qt3to4:
#include <Q3ValueList>
#include <Q3CString>

#include <kcharsets.h>
#include <kdebug.h>
#include <kglobal.h>

#include "diffsettings.h"


KompareProcess::KompareProcess( DiffSettings* diffSettings, enum Kompare::DiffMode mode, const QString& source, const QString& destination, const QString& dir )
	: m_diffSettings( diffSettings ),
	m_mode( mode ),
	m_textDecoder( 0 ),
	m_proc(new QProcess)
{

	// connect the stdout and stderr signals
	connect( m_proc, SIGNAL( readyReadStandardOutput() ),
		SLOT  ( slotReceivedStdout( ) ) );
	connect( this, SIGNAL( readyReadStandardError() ),
		SLOT  ( slotReceivedStderr( ) ) );

	// connect the signal that indicates that the proces has exited
	connect( this, SIGNAL( finished( int, QProcess::ExitStatus ) ),
		SLOT  ( slotProcessExited( int, QProcess::ExitStatus ) ) );

	m_env << "LANG=C";

	// Write command and options
	if( m_mode == Kompare::Default )
	{
		writeDefaultCommandLine();
	}
	else
	{
		writeCommandLine();
	}

	if( !dir.isEmpty() ) {
		m_proc->setWorkingDirectory( dir );
	}

	// Write file names
	m_args << "--";
	m_args << constructRelativePath( dir, source );
	m_args << constructRelativePath( dir, destination );
}

void KompareProcess::writeDefaultCommandLine()
{
	if ( !m_diffSettings || m_diffSettings->m_diffProgram.isEmpty() )
	{
	m_prog = "diff";
		m_args << "-dr";
	}
	else
	{
	m_prog = m_diffSettings->m_diffProgram;
		m_args  << "-dr";
	}

	m_args << "-U" << QString::number( m_diffSettings->m_linesOfContext );
}

void KompareProcess::writeCommandLine()
{
	// load the executable into the K3Process
	if ( m_diffSettings->m_diffProgram.isEmpty() )
	{
		kDebug(8101) << "Using the first diff in the path..." << endl;
		m_prog =  "diff";
	}
	else
	{
		kDebug(8101) << "Using a user specified diff, namely: " << m_diffSettings->m_diffProgram << endl;
		m_prog = m_diffSettings->m_diffProgram;
	}

	switch( m_diffSettings->m_format ) {
	case Kompare::Unified :
		m_args << "-U" << QString::number( m_diffSettings->m_linesOfContext );
		break;
	case Kompare::Context :
		m_args << "-C" << QString::number( m_diffSettings->m_linesOfContext );
		break;
	case Kompare::RCS :
		m_args << "-n";
		break;
	case Kompare::Ed :
		m_args << "-e";
		break;
	case Kompare::SideBySide:
		m_args << "-y";
		break;
	case Kompare::Normal :
	case Kompare::UnknownFormat :
	default:
		break;
	}

	if ( m_diffSettings->m_largeFiles
// default diff does not have -H on OpenBSD
// so don't pass this option unless the user overrode the default program
#if defined(__OpenBSD__)
		&& !m_diffSettings->m_diffProgram.isEmpty()
#endif
	   )
	{
		m_args << "-H";
	}

	if ( m_diffSettings->m_ignoreWhiteSpace )
	{
		m_args << "-b";
	}

	if ( m_diffSettings->m_ignoreAllWhiteSpace )
	{
		m_args << "-w";
	}

	if ( m_diffSettings->m_ignoreEmptyLines )
	{
		m_args << "-B";
	}

	if ( m_diffSettings->m_ignoreChangesDueToTabExpansion )
	{
		m_args << "-E";
	}

	if ( m_diffSettings->m_createSmallerDiff )
	{
		m_args << "-d";
	}

	if ( m_diffSettings->m_ignoreChangesInCase )
	{
		m_args << "-i";
	}

	if ( m_diffSettings->m_ignoreRegExp && !m_diffSettings->m_ignoreRegExpText.isEmpty() )
	{
		m_args << "-I " << m_diffSettings->m_ignoreRegExpText;
	}

	if ( m_diffSettings->m_showCFunctionChange )
	{
		m_args << "-p";
	}

	if ( m_diffSettings->m_convertTabsToSpaces )
	{
		m_args << "-t";
	}

	if ( m_diffSettings->m_recursive )
	{
		m_args << "-r";
	}

	if ( m_diffSettings->m_newFiles )
	{
		m_args << "-N";
	}

// This option is more trouble than it is worth... please do not ever enable it unless you want really weird crashes
//	if ( m_diffSettings->m_allText )
//	{
//		*this << "-a";
//	}

	if ( m_diffSettings->m_excludeFilePattern )
	{
		QStringList::ConstIterator it = m_diffSettings->m_excludeFilePatternList.begin();
		QStringList::ConstIterator end = m_diffSettings->m_excludeFilePatternList.end();
		for ( ; it != end; ++it )
		{
			m_args << "-x" << *it ;
		}
	}

	if ( m_diffSettings->m_excludeFilesFile && !m_diffSettings->m_excludeFilesFileURL.isEmpty() )
	{
		m_args << "-X" << m_diffSettings->m_excludeFilesFileURL;
	}
}

KompareProcess::~KompareProcess()
{
	delete m_proc;
}

void KompareProcess::setEncoding( const QString& encoding )
{
	QTextCodec* textCodec = KGlobal::charsets()->codecForName( encoding.toLatin1() );
	if ( textCodec )
		m_textDecoder = textCodec->makeDecoder();
	else
	{
		kDebug(8101) << "Using locale codec as backup..." << endl;
		textCodec = QTextCodec::codecForLocale();
		m_textDecoder = textCodec->makeDecoder();
	}
}

void KompareProcess::slotReceivedStdout( )
{
	m_stdout += m_proc->readAllStandardOutput();
}

void KompareProcess::slotReceivedStderr( )
{
	m_stderr += m_proc->readAllStandardError();
}

void KompareProcess::start()
{
#ifndef NDEBUG
	QString cmdLine;
	QStringList::ConstIterator it = m_args.begin();
	for (; it != m_args.end(); ++it )
		cmdLine += "\"" + (*it) + "\" ";
	kDebug(8101) << cmdLine << endl;
#endif
	m_proc->setEnvironment(m_env);
	m_proc->start(m_prog, m_args);
}

QProcess* KompareProcess::process()
{
	return m_proc;
}

void KompareProcess::slotProcessExited( int code, QProcess::ExitStatus status )
{
	// exit status of 0: no differences
	//   1: some differences
	//   2: error but there may be differences !
	kDebug(8101) << "Exited with exit status : " << status << endl;
	emit diffHasFinished( code == 0 && status != QProcess::CrashExit );
}

#include "kompareprocess.moc"

