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
#ifndef AUTOTOOLSACTION_H
#define AUTOTOOLSACTION_H

#include <kaction.h>

class QObject;
class KActionCollection;

/**
 * A KAction derivative that will work with the QToolButtons used in
 * the Automake Manager
 * @author Matt Rogers
 */
class AutoToolsAction : public KAction
{
public:
	virtual ~AutoToolsAction();

	AutoToolsAction( const QString& text, const KShortcut& cut,
	                 const QObject* receiver, const char* slot,
	                 KActionCollection* parent, const char* name );

	AutoToolsAction( const QString& text, const QIconSet& pix, const KShortcut& cut,
	                 const QObject* receiver, const char* slot,
	                 KActionCollection* parent, const char* name );

	AutoToolsAction( const QString& text, const QString& pix, const KShortcut& cut,
	                 const QObject* receiver, const char* slot,
	                 KActionCollection* parent, const char* name );

	AutoToolsAction( const KGuiItem& item, const KShortcut& cut,
	                 const QObject* receiver, const char* slot,
	                 KActionCollection* parent, const char* name );

	AutoToolsAction( const QString& text, const KShortcut& cut = KShortcut(), QObject* parent = 0, const char* name = 0 );
	AutoToolsAction( const QString& text, const KShortcut& cut,
	                 const QObject* receiver, const char* slot, QObject* parent, const char* name = 0 );
	AutoToolsAction( const QString& text, const QIconSet& pix, const KShortcut& cut = KShortcut(),
	                 QObject* parent = 0, const char* name = 0 );
	AutoToolsAction( const QString& text, const QString& pix, const KShortcut& cut = KShortcut(),
	                 QObject* parent = 0, const char* name = 0 );
	AutoToolsAction( const QString& text, const QIconSet& pix, const KShortcut& cut,
	                 const QObject* receiver, const char* slot, QObject* parent, const char* name = 0 );
	AutoToolsAction( const QString& text, const QString& pix, const KShortcut& cut,
	                 const QObject* receiver, const char* slot, QObject* parent,
	                 const char* name = 0 );
	AutoToolsAction( QObject* parent = 0, const char* name = 0 );

	virtual int plug( QWidget *widget, int index = -1 );

	virtual void updateEnabled( int i );
};

#endif

//kate: indent-mode csands; tab-width 4; auto-insert-doxygen on;