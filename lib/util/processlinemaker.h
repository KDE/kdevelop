/***************************************************************************
 *   Copyright (C) 2002 John Firebaugh <jfirebaugh@kde.org                 *
 *                                                                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _PROCESSLINEMAKER_H_
#define _PROCESSLINEMAKER_H_

#include <qobject.h>

class KProcess;

class ProcessLineMaker : public QObject
{
Q_OBJECT

public:
	ProcessLineMaker();
	ProcessLineMaker( const KProcess* );

public slots:
	void slotReceivedStdout(const QString&);
	void slotReceivedStderr(const QString&);

	void slotReceivedStdout(const char*);
	void slotReceivedStderr(const char*);

protected slots:
	void slotReceivedStdout(KProcess *, char *buffer, int buflen);
	void slotReceivedStderr(KProcess *, char *buffer, int buflen);
        
signals:
	void receivedStdoutLine( const QString& line );
	void receivedStderrLine( const QString& line );
        
private:
	QString stdoutbuf;
	QString stderrbuf;
};

#endif
