/***************************************************************************
 *   Copyright (C) 2005 by Jens Dagerbo                                    *
 *   jens.dagerbo@swipnet.se                                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qlineedit.h>

#include <kapplication.h>
#include <kurlcompletion.h>
#include <kurlrequester.h>
#include <kconfig.h>
#include <kurl.h>

#include <domutil.h>

#include "ctags2_settingswidget.h"
#include "ctags2_part.h"

CTags2SettingsWidget::CTags2SettingsWidget( CTags2Part * part, QWidget* parent, const char* name, WFlags fl )
	: CTags2SettingsWidgetBase( parent, name, fl ), m_part( part )
{
	binaryPath->completionObject()->setMode( KURLCompletion::FileCompletion );
	binaryPath->setMode( KFile::File | KFile::LocalOnly );
	binaryPath->setShowLocalProtocol( false );

	tagfilePath->completionObject()->setMode( KURLCompletion::FileCompletion );
	tagfilePath->setMode( KFile::File | KFile::ExistingOnly | KFile::LocalOnly );
	tagfilePath->setShowLocalProtocol( false );
	
	loadSettings();
}

CTags2SettingsWidget::~CTags2SettingsWidget()
{
}

void CTags2SettingsWidget::loadSettings()
{
	QDomDocument & dom = *m_part->projectDom();
	showTypeBox->setChecked( DomUtil::readBoolEntry( dom, "/ctagspart/showType", true ) );
	showDefinitionBox->setChecked( DomUtil::readBoolEntry( dom, "/ctagspart/showDefinition", true ) );
	showDeclarationBox->setChecked( DomUtil::readBoolEntry( dom, "/ctagspart/showDeclaration", true ) );
	showLookupBox->setChecked( DomUtil::readBoolEntry( dom, "/ctagspart/showLookup", true ) );
	tagfileDefault->setChecked( DomUtil::readBoolEntry( dom, "/ctagspart/tagfileDefault", true ) );
	tagfileCustomEdit->setText( DomUtil::readEntry( dom, "/ctagspart/customArguments" ) );
	tagfilePath->setURL( DomUtil::readEntry( dom, "/ctagspart/customTagfilePath" ) );
	
	KConfig * config = kapp->config();
	config->setGroup( "CTAGS" );
	binaryPath->setURL( config->readEntry( "ctags binary" ) );
}

void CTags2SettingsWidget::storeSettings()
{
	QDomDocument & dom = *m_part->projectDom();
	DomUtil::writeBoolEntry( dom, "/ctagspart/showType", showTypeBox->isChecked() );
	DomUtil::writeBoolEntry( dom, "/ctagspart/showDefinition", showDefinitionBox->isChecked() );
	DomUtil::writeBoolEntry( dom, "/ctagspart/showDeclaration", showDeclarationBox->isChecked() );
	DomUtil::writeBoolEntry( dom, "/ctagspart/showLookup", showLookupBox->isChecked() );
	DomUtil::writeBoolEntry( dom, "/ctagspart/tagfileDefault", tagfileDefault->isChecked() );
	DomUtil::writeEntry( dom, "/ctagspart/customArguments", tagfileCustomEdit->text() );
	DomUtil::writeEntry( dom, "/ctagspart/customTagfilePath", tagfilePath->url() );
	
	KConfig * config = kapp->config();
	config->setGroup( "CTAGS" );
	config->writeEntry( "ctags binary", binaryPath->url() );
	
	emit newTagsfileName( tagfilePath->url() );
}

void CTags2SettingsWidget::slotAccept( )
{
	storeSettings();
}



#include "ctags2_settingswidget.moc"

