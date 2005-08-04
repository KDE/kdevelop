/***************************************************************************
 *   Copyright (C) 2003 by Jens Dagerbo                                    *
 *   jens.dagerbo@swipnet.se                                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qspinbox.h>
#include <qlineedit.h>

#include "bookmarks_part.h"
#include "bookmarks_config.h"
#include "bookmarks_settings.h"


BookmarkSettings::BookmarkSettings( BookmarksPart * part, QWidget* parent, const char* name, Qt::WFlags fl )
: BookmarkSettingsBase( parent, name, fl ), m_part( part )
{
	m_part->config()->readConfig();
	
	if ( m_part->config()->codeline() == BookmarksConfig::Never )
	{
		radioButton1->setChecked( true );
	}
	else if ( m_part->config()->codeline() == BookmarksConfig::Token )
	{
		radioButton2->setChecked( true );
	}
	else 
	{
		radioButton3->setChecked( true );
	}
	
	checkBox1->setChecked( m_part->config()->toolTip() );
	spinBox1->setValue( m_part->config()->context() );
	lineEdit1->setText( m_part->config()->token() );
}

BookmarkSettings::~BookmarkSettings()
{
}

void BookmarkSettings::slotAccept()
{
	BookmarksConfig::CodeLineType codeline = BookmarksConfig::Never;
	codeline = radioButton1->isChecked() ? BookmarksConfig::Never : codeline;
	codeline = radioButton2->isChecked() ? BookmarksConfig::Token : codeline;
	codeline = radioButton3->isChecked() ? BookmarksConfig::Always : codeline;
	m_part->config()->setCodeline( codeline );
	m_part->config()->setToolTip( checkBox1->isChecked() );
	m_part->config()->setContext( spinBox1->value() );
	m_part->config()->setToken( lineEdit1->text() );
	
	m_part->config()->writeConfig();
}


#include "bookmarks_settings.moc"

// kate: space-indent off; indent-width 4; tab-width 4; show-tabs off;
