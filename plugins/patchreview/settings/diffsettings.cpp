/***************************************************************************
                                diffsettings.cpp
                                ----------------
        begin                   : Sun Mar 4 2001
        Copyright 2001-2004 Otto Bruggeman <otto.bruggeman@home.nl>
        Copyright 2007      Kevin Kofler   <kevin.kofler@chello.at>
****************************************************************************/

/***************************************************************************
**
**   This program is free software; you can redistribute it and/or modify
**   it under the terms of the GNU General Public License as published by
**   the Free Software Foundation; either version 2 of the License, or
**   (at your option) any later version.
**
****************************************************************************/

#include "diffsettings.h"

#include <kconfig.h>
#include <kconfiggroup.h>

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
{
	KConfigGroup group( config, "Diff Options" );
	m_diffProgram                    = group.readEntry    ( "DiffProgram", "" );
	m_linesOfContext                 = group.readEntry ( "LinesOfContext", 3 );
	m_largeFiles                     = group.readEntry( "LargeFiles", true );
	m_ignoreWhiteSpace               = group.readEntry( "IgnoreWhiteSpace", false );
	m_ignoreAllWhiteSpace            = group.readEntry( "IgnoreAllWhiteSpace", false );
	m_ignoreEmptyLines               = group.readEntry( "IgnoreEmptyLines", false );
	m_ignoreChangesDueToTabExpansion = group.readEntry( "IgnoreChangesDueToTabExpansion", false );
	m_ignoreChangesInCase            = group.readEntry( "IgnoreChangesInCase", false );
	m_ignoreRegExp                   = group.readEntry( "IgnoreRegExp", false );
	m_ignoreRegExpText               = group.readEntry    ( "IgnoreRegExpText", "" );
	m_ignoreRegExpTextHistory        = group.readEntry( "IgnoreRegExpTextHistory", QStringList() );
	m_createSmallerDiff              = group.readEntry( "CreateSmallerDiff", true );
	m_convertTabsToSpaces            = group.readEntry( "ConvertTabsToSpaces", false );
	m_showCFunctionChange            = group.readEntry( "ShowCFunctionChange", false );
	m_recursive                      = group.readEntry( "CompareRecursively", true );
	m_newFiles                       = group.readEntry( "NewFiles", true );

	m_format = static_cast<Kompare::Format>( group.readEntry( "Format", (int) Kompare::Unified ) );

	KConfigGroup group2 ( config, "Exclude File Options" );
	m_excludeFilePattern             = group2.readEntry( "Pattern", false );
	m_excludeFilePatternList         = group2.readEntry( "PatternList", QStringList() );
	m_excludeFilesFile               = group2.readEntry( "File", false );
	m_excludeFilesFileURL            = group2.readEntry    ( "FileURL", "" );
	m_excludeFilesFileHistoryList    = group2.readEntry( "FileHistoryList", QStringList() );
}

void DiffSettings::saveSettings( KConfig* config )
{
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

	KConfigGroup group2( config, "Exclude File Options" );
	group2.writeEntry( "Pattern",            m_excludeFilePattern );
	group2.writeEntry( "PatternList",        m_excludeFilePatternList );
	group2.writeEntry( "File",               m_excludeFilesFile );
	group2.writeEntry( "FileURL",            m_excludeFilesFileURL );
	group2.writeEntry( "FileHistoryList",    m_excludeFilesFileHistoryList );

	config->sync();
}

#include "diffsettings.moc"
