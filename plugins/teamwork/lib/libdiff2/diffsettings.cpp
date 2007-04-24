/***************************************************************************
				diffsettings.cpp  -  description
				-------------------
	begin			: Sun Mar 4 2001
	copyright		: (C) 2001-2004 Otto Bruggeman
	email			: otto.bruggeman@home.nl
****************************************************************************/

/***************************************************************************
**
**   This program is free software; you can redistribute it and/or modify
**   it under the terms of the GNU General Public License as published by
**   the Free Software Foundation; either version 2 of the License, or
**   (at your option) any later version.
**
****************************************************************************/

#include <kconfig.h>

#include "diffsettings.h"

DiffSettings::DiffSettings( QWidget* parent )
	: SettingsBase( parent ),
	m_linesOfContext( 0 ),
	m_format( Kompare::Unified ),
	m_largeFiles( false ),
	m_ignoreWhiteSpace( false ),
	m_ignoreAllWhiteSpace( false ),
	m_ignoreEmptyLines( false ),
	m_ignoreChangesDueToTabExpansion( false ),
	m_createSmallerDiff( false ),
	m_ignoreChangesInCase( false ),
	m_showCFunctionChange( false ),
	m_convertTabsToSpaces( false ),
	m_ignoreRegExp( false ),
	m_recursive( false ),
	m_newFiles( false ),
	m_excludeFilePattern( false ),
	m_excludeFilesFile( false )
{
}

DiffSettings::~DiffSettings()
{
}

void DiffSettings::loadSettings( KConfig* config )
{/*
	KConfigGroup group( config, "Diff Options" );
	m_diffProgram                    = group.readEntry    ( "DiffProgram", "" );
	m_linesOfContext                 = group.readEntry( "LinesOfContext", QVariant(3) ).asInt();
	m_largeFiles                     = group.readBoolEntry( "LargeFiles", true );
	m_ignoreWhiteSpace               = group.readBoolEntry( "IgnoreWhiteSpace", false );
	m_ignoreAllWhiteSpace            = group.readBoolEntry( "IgnoreAllWhiteSpace", false );
	m_ignoreEmptyLines               = group.readBoolEntry( "IgnoreEmptyLines", false );
	m_ignoreChangesDueToTabExpansion = group.readBoolEntry( "IgnoreChangesDueToTabExpansion", false );
	m_ignoreChangesInCase            = group.readBoolEntry( "IgnoreChangesInCase", false );
	m_ignoreRegExp                   = group.readBoolEntry( "IgnoreRegExp", false );
	m_ignoreRegExpText               = group.readEntry    ( "IgnoreRegExpText", "" );
	m_ignoreRegExpTextHistory        = group.readListEntry( "IgnoreRegExpTextHistory" );
	m_createSmallerDiff              = group.readBoolEntry( "CreateSmallerDiff", true );
	m_convertTabsToSpaces            = group.readBoolEntry( "ConvertTabsToSpaces", false );
	m_showCFunctionChange            = group.readBoolEntry( "ShowCFunctionChange", false );
	m_recursive                      = group.readBoolEntry( "CompareRecursively", true );
	m_newFiles                       = group.readBoolEntry( "NewFiles", true );

	m_format = static_cast<Kompare::Format>( group.readNumEntry( "Format", Kompare::Unified ) );

	KConfigGroup group2 ( config, "Exclude File Options" );
	m_excludeFilePattern             = group2.readBoolEntry( "Pattern", false );
	m_excludeFilePatternList         = group2.readListEntry( "PatternList" );
	m_excludeFilesFile               = group2.readBoolEntry( "File", false );
	m_excludeFilesFileURL            = group2.readEntry    ( "FileURL", "" );
	m_excludeFilesFileHistoryList    = group2.readListEntry( "FileHistoryList" );*/
}

void DiffSettings::saveSettings( KConfig* config )
{/*
	KConfigGroup group( config, "Diff Options" );
	group.writeEntry( "DiffProgram",                    m_diffProgram );
	group.writeEntry( "LinesOfContext",                 m_linesOfContext );
	group.writeEntry( "Format",                         QString(m_format) );
	group.writeEntry( "LargeFiles",                     m_largeFiles );
	group.writeEntry( "IgnoreWhiteSpace",               m_ignoreWhiteSpace );
	group.writeEntry( "IgnoreAllWhiteSpace",            m_ignoreAllWhiteSpace );
	group.writeEntry( "IgnoreEmptyLines",               m_ignoreEmptyLines );
	group.writeEntry( "IgnoreChangesInCase",            m_ignoreChangesInCase );
	group.writeEntry( "IgnoreChangesDueToTabExpansion", m_ignoreChangesDueToTabExpansion );
	group.writeEntry( "IgnoreRegExp",                   m_ignoreRegExp );
	group.writeEntry( "IgnoreRegExpText",               m_ignoreRegExpText );
	group.writeEntry( "IgnoreRegExpTextHistory",        m_ignoreRegExpTextHistory );
	group.writeEntry( "CreateSmallerDiff",              m_createSmallerDiff );
	group.writeEntry( "ConvertTabsToSpaces",            m_convertTabsToSpaces );
	group.writeEntry( "ShowCFunctionChange",            m_showCFunctionChange );
	group.writeEntry( "CompareRecursively",             m_recursive );
	group.writeEntry( "NewFiles",                       m_newFiles );
	group.setDirty( true );

	KConfigGroup group2( config, "Exclude File Options" );
	group2.writeEntry( "Pattern",            m_excludeFilePattern );
	group2.writeEntry( "PatternList",        m_excludeFilePatternList );
	group2.writeEntry( "File",               m_excludeFilesFile );
	group2.writeEntry( "FileURL",            m_excludeFilesFileURL );
	group2.writeEntry( "FileHistoryList",    m_excludeFilesFileHistoryList );
	group2.setDirty( true );*/
}

#include "diffsettings.moc"
