/***************************************************************************
                                kompareprocess.h  -  description
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

#ifndef KOMPAREPROCESS_H
#define KOMPAREPROCESS_H

#include "kompare.h"
#include <QtCore/QProcess>

class QTextCodec;
class QTextDecoder;
class QProcess;

class DiffSettings;

class KompareProcess : public QObject, public KompareFunctions
{
	Q_OBJECT

public:
	KompareProcess( DiffSettings* diffSettings, enum Kompare::DiffMode mode, const QString& source, const QString& destination, const QString& directory = QString() );
	~KompareProcess();

	void start();

	QProcess* process();

	QString diffOutput() { return m_stdout; }
	QString stdOut()	 { return m_stdout; }
	QString stdErr()	 { return m_stderr; }

	void setEncoding( const QString& encoding );

signals:
	void diffHasFinished( bool finishedNormally );

protected:
	void writeDefaultCommandLine();
	void writeCommandLine();

protected slots:
	void slotReceivedStdout( );
	void slotReceivedStderr( );
	void slotProcessExited( int, QProcess::ExitStatus );

private:
	DiffSettings*		  m_diffSettings;
	enum Kompare::DiffMode m_mode;
	QString				m_stdout;
	QString				m_stderr;
	QTextDecoder*		  m_textDecoder;
	QProcess*			  m_proc;
	QStringList			m_env;
	QStringList			m_args;
	QString				m_prog;
};

#endif
