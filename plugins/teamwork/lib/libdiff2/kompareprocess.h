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

#include <k3process.h>

#include "kompare.h"

class QTextCodec;

class DiffSettings;

class KompareProcess : public K3Process, public KompareFunctions
{
	Q_OBJECT

public:
	KompareProcess( DiffSettings* diffSettings, enum Kompare::DiffMode mode, QString source, QString destination, QString directory = QString::null );
	~KompareProcess();

	bool start();

	QString diffOutput() { return m_stdout; }
	QString stdOut()     { return m_stdout; }
	QString stdErr()     { return m_stderr; }

	void setEncoding( const QString& encoding );

signals:
	void diffHasFinished( bool finishedNormally );

protected:
	void writeDefaultCommandLine();
	void writeCommandLine();

protected slots:
	void slotReceivedStdout( K3Process*, char*, int );
	void slotReceivedStderr( K3Process*, char*, int );
	void slotProcessExited( K3Process* proc );

private:
	DiffSettings*          m_diffSettings;
	enum Kompare::DiffMode m_mode;
	QString                m_stdout;
	QString                m_stderr;
	QTextDecoder*          m_textDecoder;
};

#endif
