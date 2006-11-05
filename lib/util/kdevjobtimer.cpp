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

#include "kdevjobtimer.h"

KDevJobTimer::KDevJobTimer( void * payload, QObject *parent, const char *name)
 : QTimer(parent, name)
{
	m_payload = payload;
	connect( this, SIGNAL(timeout()), this, SLOT(slotTimeout()) );
}

KDevJobTimer::~KDevJobTimer()
{
}

void KDevJobTimer::singleShot(int msec, QObject * receiver, const char * member, void * payload)
{
	KDevJobTimer * p = new KDevJobTimer( payload );
	p->start( msec, true );
	connect( p, SIGNAL(timeout(void*)), receiver, member );
}

void KDevJobTimer::slotTimeout()
{
	emit timeout( m_payload );
	m_payload = 0;
	deleteLater();
}

#include "kdevjobtimer.moc"
