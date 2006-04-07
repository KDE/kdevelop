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

#include <qtimer.h>
#include <qframe.h>

#include <kdebug.h>
#include <kparts/part.h>
#include <klibloader.h>
#include <kde_terminal_interface.h>
#include <kprocess.h>

#include "kdevshellwidget.h"

KDevShellWidget::KDevShellWidget(QWidget *parent, const char *name)
 : QVBox(parent, name), m_doAutoActivate( false ), m_isRunning( false )
{
}


KDevShellWidget::~KDevShellWidget()
{
}

void KDevShellWidget::setShell( const QString & shell, const QStrList & arguments )
{
	m_shellName = shell;
	m_shellArguments = arguments;
}

void KDevShellWidget::activate( )
{
	KLibFactory *factory = KLibLoader::self()->factory("libkonsolepart");
	if ( !factory ) return;

  	m_konsolePart = (KParts::ReadOnlyPart *) factory->create( this, "libkonsolepart", "KParts::ReadOnlyPart" );
	if ( !m_konsolePart ) return;

	connect( m_konsolePart, SIGNAL( processExited(KProcess *) ), this, SLOT( processExited(KProcess *) ) );
	connect( m_konsolePart, SIGNAL( receivedData( const QString& ) ), this, SIGNAL( receivedData( const QString& ) ) );
	connect( m_konsolePart, SIGNAL(destroyed()), this, SLOT(partDestroyed()) );

 	m_konsolePart->widget()->setFocusPolicy( QWidget::WheelFocus );
 	setFocusProxy( m_konsolePart->widget() );
 	m_konsolePart->widget()->setFocus();

 	if ( m_konsolePart->widget()->inherits("QFrame") )
 		((QFrame*)m_konsolePart->widget())->setFrameStyle( QFrame::Panel | QFrame::Sunken );

	m_konsolePart->widget()->show();

	TerminalInterface* ti = static_cast<TerminalInterface*>( m_konsolePart->qt_cast( "TerminalInterface" ) );
	if( !ti ) return;

	ti->startProgram( m_shellName, m_shellArguments );

	m_isRunning = true;

}

void KDevShellWidget::partDestroyed( )
{
	if ( m_doAutoActivate )
	{
		activate();
	}
}

void KDevShellWidget::processExited( KProcess * proc )
{
	m_isRunning = false;

	if ( !proc ) return;

	kdDebug(9000) << proc->args() << endl;

	if ( proc->normalExit() )
		emit shellExited( proc->exitStatus() );
	else if ( proc->signalled() )
		emit shellSignalled( proc->exitSignal() );
}

void KDevShellWidget::sendInput( const QString & text )
{
	if ( !m_konsolePart ) return;
	TerminalInterface* ti = static_cast<TerminalInterface*>( m_konsolePart->qt_cast( "TerminalInterface" ) );
	if( !ti ) return;

	ti->sendInput( text );
}

bool KDevShellWidget::isRunning( )
{
	return m_isRunning;
}

void KDevShellWidget::setAutoReactivateOnClose( bool doAutoActivate )
{
	// to auto reactivate can be dangerous, do it like this to avoid
	// reactivating with a non-working setting (the partDestroyed()
	// slot will have ran before m_doAutoActivate is set)
	if ( doAutoActivate )
		QTimer::singleShot( 3000, this, SLOT(setAutoReactivateOnCloseDelayed()) );
	else
		m_doAutoActivate = false;
}

void KDevShellWidget::setAutoReactivateOnCloseDelayed( )
{
	m_doAutoActivate = true;
}


#include "kdevshellwidget.moc"

// kate: space-indent off; indent-width 4; tab-width 4; show-tabs off;
