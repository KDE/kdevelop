//
// C++ Implementation: 
//
// Description: 
//
//
// Author: KDevelop Authors <kdevelop-devel@kdevelop.org>, (C) 2003
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qspinbox.h>

#include "bookmarks_part.h"
#include "bookmarks_config.h"
#include "bookmarks_settings.h"


BookmarkSettings::BookmarkSettings( BookmarksPart * part, QWidget* parent, const char* name, WFlags fl )
: BookmarkSettingsBase( parent, name, fl ), m_part( part )
{
	m_part->config()->readConfig();
	
	if ( m_part->config()->codeline() == 0 )
	{
		radioButton1->setChecked( true );
	}
	else if ( m_part->config()->codeline() == 1 )
	{
		radioButton2->setChecked( true );
	}
	else 
	{
		radioButton3->setChecked( true );
	}
	
	checkBox1->setChecked( m_part->config()->toolTip() );
	spinBox1->setValue( m_part->config()->context() );
}

BookmarkSettings::~BookmarkSettings()
{
}

void BookmarkSettings::slotAccept()
{
	unsigned int codeline = 0;
	codeline = radioButton1->isChecked() ? 0 : codeline;
	codeline = radioButton2->isChecked() ? 1 : codeline;
	codeline = radioButton3->isChecked() ? 2 : codeline;
	m_part->config()->setCodeline( codeline );
	m_part->config()->setToolTip( checkBox1->isChecked() );
	m_part->config()->setContext( spinBox1->value() );
	
	m_part->config()->writeConfig();
}


#include "bookmarks_settings.moc"

