/*
 *  Copyright (C) 2003 Roberto Raggi (roberto@kdevelop.org)
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 *
 */

#include "qeditor_settings.h"
#include "qeditor_factory.h"

#include <kinstance.h>
#include <kconfig.h>

QEditorSettings* QEditorSettings::m_self = 0;

QEditorSettings::QEditorSettings( KConfig* config )
{
    m_config = config;
    init();
}

QEditorSettings::~QEditorSettings()
{
}

void QEditorSettings::init()
{
    if( !m_config )
	return;

    m_config->setGroup(generalGroup());
    m_wordWrap = m_config->readBoolEntry( "WordWrap", true );
    m_tabStop = m_config->readNumEntry( "TabStop", 8 );
    m_completeWordWithSpace = m_config->readBoolEntry( "CompleteWordWithSpace", false );
    m_parenthesesMatching = m_config->readBoolEntry( "ParenthesesMatching", true );
    m_showMarkers = m_config->readBoolEntry( "ShowMarkers", true );
    m_showLineNumber = m_config->readBoolEntry( "ShowLineNumber", false );
    m_showCodeFoldingMarkers = m_config->readBoolEntry( "ShowCodeFoldingMarkers", true );
}

QEditorSettings* QEditorSettings::self()
{
    if( !m_self )
	m_self = new QEditorSettings( QEditorPartFactory::instance()->config() );

    return m_self;
}

void QEditorSettings::setWordWrap( bool enable )
{
    m_wordWrap = enable;
    
    KConfigGroupSaver cgs( m_config, generalGroup() );
    m_config->writeEntry( "WordWrap", m_wordWrap );
    m_config->sync();
}

void QEditorSettings::setTabStop( int tabStop )
{
    m_tabStop = tabStop;

    KConfigGroupSaver cgs( m_config, generalGroup() );
    m_config->writeEntry( "TabStop", m_tabStop );
    m_config->sync();
}

void QEditorSettings::setCompleteWordWithSpace( bool enable )
{
    m_completeWordWithSpace = enable;

    KConfigGroupSaver cgs( m_config, generalGroup() );
    m_config->writeEntry( "CompleteWordWithSpace", m_completeWordWithSpace );
    m_config->sync();
}

void QEditorSettings::setParenthesesMatching( bool enable )
{
    m_parenthesesMatching = enable;

    KConfigGroupSaver cgs( m_config, generalGroup() );
    m_config->writeEntry( "ParenthesesMatching", m_parenthesesMatching );
    m_config->sync();
}

void QEditorSettings::setShowMarkers( bool enable )
{
    m_showMarkers = enable;
    
    KConfigGroupSaver cgs( m_config, generalGroup() );
    m_config->writeEntry( "ShowMarkers", m_showMarkers );
    m_config->sync();    
}

void QEditorSettings::setShowLineNumber( bool enable )
{
    m_showLineNumber = enable;
    
    KConfigGroupSaver cgs( m_config, generalGroup() );
    m_config->writeEntry( "ShowLineNumber", m_showLineNumber );
    m_config->sync();    
}

void QEditorSettings::setShowCodeFoldingMarkers( bool enable )
{
    m_showCodeFoldingMarkers = enable;
    
    KConfigGroupSaver cgs( m_config, generalGroup() );
    m_config->writeEntry( "ShowCodeFoldingMarkers", m_showCodeFoldingMarkers );
    m_config->sync();    
}

