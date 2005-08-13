/*
   KDevelop Autotools Support
   Copyright (c) 2005 by Matt Rogers <mattr@kde.org>

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************
*/
#include "autotoolsaction.h"

#include <qtoolbutton.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kiconloader.h>

AutoToolsAction::~AutoToolsAction()
{
}

AutoToolsAction::AutoToolsAction( const QString& text, const KShortcut& cut,
                                  const QObject* receiver, const char* slot,
                                  KActionCollection* parent, const char* name )
: KAction( text, cut, receiver, slot, parent, name )
{
}

AutoToolsAction::AutoToolsAction( const QString& text, const QIconSet& pix, const KShortcut& cut,
                                  const QObject* receiver, const char* slot,
                                  KActionCollection* parent, const char* name )
: KAction( text, pix, cut, receiver, slot, parent, name )
{
}

AutoToolsAction::AutoToolsAction( const QString& text, const QString& pix, const KShortcut& cut,
                                  const QObject* receiver, const char* slot,
                                  KActionCollection* parent, const char* name )
: KAction( text, pix, cut, receiver, slot, parent, name )
{
}

AutoToolsAction::AutoToolsAction( const KGuiItem& item, const KShortcut & cut,
                                  const QObject* receiver, const char* slot,
                                  KActionCollection* parent, const char* name )
: KAction( item, cut, receiver, slot, parent, name )
{
}

AutoToolsAction::AutoToolsAction( const QString& text, const KShortcut& cut,
                                  QObject* parent, const char* name )
: KAction( text, cut, parent, name )
{
}

AutoToolsAction::AutoToolsAction( const QString& text, const KShortcut& cut,
                                  const QObject* receiver, const char* slot,
                                  QObject* parent, const char* name )
: KAction( text, cut, receiver, slot, parent, name )
{
}

AutoToolsAction::AutoToolsAction( const QString& text, const QIconSet& pix,
                                  const KShortcut& cut, QObject* parent, const char* name )
: KAction( text, pix, cut, parent, name )
{
}

AutoToolsAction::AutoToolsAction( const QString& text, const QString& pix,
                                  const KShortcut& cut, QObject* parent, const char* name )
: KAction( text, pix, cut, parent, name )
{
}

AutoToolsAction::AutoToolsAction( const QString& text, const QIconSet& pix,
                                  const KShortcut& cut, const QObject* receiver,
                                  const char* slot, QObject* parent, const char * name )
: KAction( text, pix, cut, receiver, slot, parent, name )
{
}

AutoToolsAction::AutoToolsAction( const QString& text, const QString& pix,
                                  const KShortcut & cut, const QObject* receiver,
                                  const char* slot, QObject* parent, const char * name )
: KAction( text, pix, cut, receiver, slot, parent, name )
{
}

AutoToolsAction::AutoToolsAction( QObject * parent, const char * name )
: KAction( parent, name )
{
}

int AutoToolsAction::plug( QWidget* w, int index )
{
	if ( !w ) {
		kdWarning(129) << "KAction::plug called with 0 argument\n";
		return -1;
	}
	
	// Check if action is permitted
	if (kapp && !kapp->authorizeKAction(name()))
		return -1;
	
	if ( ::qt_cast<QToolButton*>( w ) )
	{
		QToolButton* tb = static_cast<QToolButton*>( w );
		connect( tb, SIGNAL( clicked() ), this, SLOT( activate() ) );
		int id = getToolButtonID();
		
		if ( !icon().isEmpty() )
			tb->setPixmap( SmallIcon( icon() ) );
		else
			tb->setText( text() );
		
		if ( !isEnabled() )
			tb->setEnabled( false );
		
		if ( !whatsThis().isEmpty() )
		{
			QWhatsThis::remove( tb );
			QWhatsThis::add( tb, whatsThisWithIcon() );
		}
		
		if ( !toolTip().isEmpty() )
		{
			QToolTip::remove( tb );
			QToolTip::add( tb, toolTip() );
		}
		
		addContainer( tb, id );
		
		return containerCount() - 1;
	}
	
	return KAction::plug( w, index );
}

void AutoToolsAction::updateEnabled( int i )
{
	QWidget* w = container( i );
	
	if ( ::qt_cast<QToolButton*>( w ) )
		static_cast<QToolButton*>( w )->setEnabled( isEnabled() );
	else
		KAction::updateEnabled( i ) ;
}



//kate: indent-mode csands; tab-width 4;
