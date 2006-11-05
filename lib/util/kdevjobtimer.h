/***************************************************************************
 *   Copyright (C) 2006 by Jens Dagerbo                                    *
 *   jens.dagerbo@swipnet.se                                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KDEVJOBTIMER_H
#define KDEVJOBTIMER_H

#include <qtimer.h>

class KDevJobTimer : public QTimer
{
Q_OBJECT
public:
	static void singleShot( int msec, QObject * receiver, const char * member, void * payload );

signals:
	void timeout(void*);

private:
	KDevJobTimer( void * payload, QObject *parent = 0, const char *name = 0);
	~KDevJobTimer();

private slots:
	void slotTimeout();

private:
	void * m_payload;

};


#endif
