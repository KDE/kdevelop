/***************************************************************************
                                kompareprocess.cpp  -  description
                                -------------------
        begin                   : Sun Mar 4 2001
        copyright               : (C) 2001-2003 by Otto Bruggeman
                                  and John Firebaugh
        email                   : otto.bruggeman@home.nl
                                  jfirebaugh@kde.org
****************************************************************************/

/***************************************************************************
**
**   This program is free software; you can redistribute it and/or modify
**   it under the terms of the GNU General Public License as published by
**   the Free Software Foundation; either version 2 of the License, or
**   (at your option) any later version.
**
***************************************************************************/

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
#include "kompareprocess.h"

KompareProcess::KompareProcess( DiffSettings* diffSettings, enum Kompare::DiffMode mode, QString source, QString destination, QString dir )
	: K3Process(),
	m_diffSettings( diffSettings ),
	m_mode( mode ),
	m_textDecoder( 0 )
{
	setUseShell( true );

	// connect the stdout and stderr signals
	connect( this, SIGNAL( receivedStdout( K3Process*, char*, int ) ),
	         SLOT  ( slotReceivedStdout( K3Process*, char*, int ) ) );
	connect( this, SIGNAL( receivedStderr( K3Process*, char*, int ) ),
	         SLOT  ( slotReceivedStderr( K3Process*, char*, int ) ) );

	// connect the signal that indicates that the proces has exited
	connect( this, SIGNAL( processExited( K3Process* ) ),
	         SLOT  ( slotProcessExited( K3Process* ) ) );

	*this << "LANG=C";

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
		QDir::setCurrent( dir );
	}

	// Write file names
	*this << "--";
	*this << K3Process::quote( constructRelativePath( dir, source ) );
	*this << K3Process::quote( constructRelativePath( dir, destination ) );
}

void KompareProcess::writeDefaultCommandLine()
{
	if ( !m_diffSettings || m_diffSettings->m_diffProgram.isEmpty() )
	{
		*this << "diff" << "-dr";
	}
	else
	{
		*this << m_diffSettings->m_diffProgram << "-dr";
	}

	*this << "-U" << QString::number( m_diffSettings->m_linesOfContext );
}

void KompareProcess::writeCommandLine()
{
	// load the executable into the K3Process
	if ( m_diffSettings->m_diffProgram.isEmpty() )
	{
		kDebug(8101) << "Using the first diff in the path..." << endl;
		*this << "diff";
	}
	else
	{
		kDebug(8101) << "Using a user specified diff, namely: " << m_diffSettings->m_diffProgram << endl;
		*this << m_diffSettings->m_diffProgram;
	}

	switch( m_diffSettings->m_format ) {
	case Kompare::Unified :
		*this << "-U" << QString::number( m_diffSettings->m_linesOfContext );
		break;
	case Kompare::Context :
		*this << "-C" << QString::number( m_diffSettings->m_linesOfContext );
		break;
	case Kompare::RCS :
		*this << "-n";
		break;
	case Kompare::Ed :
		*this << "-e";
		break;
	case Kompare::SideBySide:
		*this << "-y";
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
		*this << "-H";
	}

	if ( m_diffSettings->m_ignoreWhiteSpace )
	{
		*this << "-b";
	}

	if ( m_diffSettings->m_ignoreAllWhiteSpace )
	{
		*this << "-w";
	}

	if ( m_diffSettings->m_ignoreEmptyLines )
	{
		*this << "-B";
	}

	if ( m_diffSettings->m_ignoreChangesDueToTabExpansion )
	{
		*this << "-E";
	}

	if ( m_diffSettings->m_createSmallerDiff )
	{
		*this << "-d";
	}

	if ( m_diffSettings->m_ignoreChangesInCase )
	{
		*this << "-i";
	}

	if ( m_diffSettings->m_ignoreRegExp && !m_diffSettings->m_ignoreRegExpText.isEmpty() )
	{
		*this << "-I " << K3Process::quote( m_diffSettings->m_ignoreRegExpText );
	}

	if ( m_diffSettings->m_showCFunctionChange )
	{
		*this << "-p";
	}

	if ( m_diffSettings->m_convertTabsToSpaces )
	{
		*this << "-t";
	}

	if ( m_diffSettings->m_recursive )
	{
		*this << "-r";
	}

	if ( m_diffSettings->m_newFiles )
	{
		*this << "-N";
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
			*this << "-x" << K3Process::quote( *it );
		}
	}

	if ( m_diffSettings->m_excludeFilesFile && !m_diffSettings->m_excludeFilesFileURL.isEmpty() )
	{
		*this << "-X" << K3Process::quote( m_diffSettings->m_excludeFilesFileURL );
	}
}

KompareProcess::~KompareProcess()
{
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

void KompareProcess::slotReceivedStdout( K3Process* /* process */, char* buffer, int length )
{
	// add all output to m_stdout
	if ( m_textDecoder )
		m_stdout += m_textDecoder->toUnicode( buffer, length );
	else
		kDebug(8101) << "KompareProcess::slotReceivedStdout : No decoder !!!" << endl;
}

void KompareProcess::slotReceivedStderr( K3Process* /* process */, char* buffer, int length )
{
	// add all output to m_stderr
	if ( m_textDecoder )
		m_stderr += m_textDecoder->toUnicode( buffer, length );
	else
		kDebug(8101) << "KompareProcess::slotReceivedStderr : No decoder !!!" << endl;
}

bool KompareProcess::start()
{
#ifndef NDEBUG
	QString cmdLine;
	QList<QByteArray>::ConstIterator it = arguments.begin();
	for (; it != arguments.end(); ++it )
	    cmdLine += "\"" + (*it) + "\" ";
	kDebug(8101) << cmdLine << endl;
#endif
	return( K3Process::start( K3Process::NotifyOnExit, K3Process::AllOutput ) );
}

void KompareProcess::slotProcessExited( K3Process* /* proc */ )
{
	// exit status of 0: no differences
	//                1: some differences
	//                2: error but there may be differences !
	kDebug(8101) << "Exited with exit status : " << exitStatus() << endl;
	emit diffHasFinished( normalExit() && exitStatus() != 0 );
}

#include "kompareprocess.moc"

