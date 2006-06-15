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
#include <qlistview.h>
#include <qheader.h>

#include <klineedit.h>
#include <kdevproject.h>
#include <kapplication.h>
#include <kurlcompletion.h>
#include <kurlrequester.h>
#include <kconfig.h>
#include <kurl.h>
#include <kdevplugin.h>
#include <domutil.h>

#include "ctags2_settingswidget.h"
#include "ctags2_part.h"
#include "ctags2_selecttagfile.h"
#include "ctags2_createtagfile.h"


CTags2SettingsWidget::CTags2SettingsWidget( CTags2Part * part, QWidget* parent, const char* name, WFlags fl )
	: CTags2SettingsWidgetBase( parent, name, fl ), m_part( part )
{
	binaryPath->completionObject()->setMode( KURLCompletion::FileCompletion );
	binaryPath->setMode( KFile::File | KFile::LocalOnly );
	binaryPath->setShowLocalProtocol( false );

	tagfilePath->completionObject()->setMode( KURLCompletion::FileCompletion );
	tagfilePath->setMode( KFile::File | KFile::ExistingOnly | KFile::LocalOnly );
	tagfilePath->setShowLocalProtocol( false );

	otherTagFiles->setSorting( -1 );
	otherTagFiles->addColumn( "" );
	otherTagFiles->header()->hide();
//    otherTagFiles->setFullWidth( true );

	loadSettings();
}

CTags2SettingsWidget::~CTags2SettingsWidget()
{
}

void CTags2SettingsWidget::loadSettings()
{
	QDomDocument & dom = *m_part->projectDom();
	tagfileCustomEdit->setText( DomUtil::readEntry( dom, "/ctagspart/customArguments" ) );
	QString customTagfile = DomUtil::readEntry( dom, "/ctagspart/customTagfilePath" );
	if (customTagfile.isEmpty())
	{
		customTagfile =  m_part->project()->projectDirectory() + "/tags";
	}
	tagfilePath->setURL(customTagfile);

	QStringList activeTagsFiles = DomUtil::readListEntry(dom, "/ctagspart/activeTagsFiles", "file");

	KConfig * config = kapp->config();
	config->setGroup( "CTAGS" );
	showDeclarationBox->setChecked( config->readBoolEntry( "ShowDeclaration", true ) );
	showDefinitionBox->setChecked( config->readBoolEntry( "ShowDefinition", true ) );
	showLookupBox->setChecked( config->readBoolEntry( "ShowLookup", true ) );
	jumpToFirstBox->setChecked( config->readBoolEntry( "JumpToFirst", false ) );
	binaryPath->setURL( config->readEntry( "ctags binary" ) );

	config->setGroup( "CTAGS-tagsfiles" );
	QMap<QString,QString> entryMap = config->entryMap( "CTAGS-tagsfiles" );
	QMap<QString,QString>::const_iterator it = entryMap.begin();
    while ( it != entryMap.end() )
	{
		QString file = config->readPathEntry( it.key() );
		new TagsItem( otherTagFiles, it.key(), file, activeTagsFiles.contains( file ) );
		++it;
	}
}

void CTags2SettingsWidget::storeSettings()
{
	QDomDocument & dom = *m_part->projectDom();
	DomUtil::writeEntry( dom, "/ctagspart/customArguments", tagfileCustomEdit->text() );
	DomUtil::writeEntry( dom, "/ctagspart/customTagfilePath", tagfilePath->url() );

	KConfig * config = kapp->config();
	config->setGroup( "CTAGS" );
	config->writeEntry( "ShowDeclaration", showDeclarationBox->isChecked() );
	config->writeEntry( "ShowDefinition", showDefinitionBox->isChecked() );
	config->writeEntry( "ShowLookup", showLookupBox->isChecked() );
	config->writeEntry( "JumpToFirst", jumpToFirstBox->isChecked() );
	config->writeEntry( "ctags binary", binaryPath->url() );

	config->deleteGroup( "CTAGS-tagsfiles" );
	config->setGroup( "CTAGS-tagsfiles" );

	QStringList activeTagsFiles;
	TagsItem * item = static_cast<TagsItem*>( otherTagFiles->firstChild() );
	while ( item )
	{
		config->writePathEntry( item->name(), item->tagsfilePath() );
		if ( item->isOn() )
		{
			activeTagsFiles.append( item->tagsfilePath() );
		}
		item = static_cast<TagsItem*>( item->nextSibling() );
	}
	DomUtil::writeListEntry( dom, "/ctagspart/activeTagsFiles", "file", activeTagsFiles );

	activeTagsFiles.push_front( tagfilePath->url() );
	Tags::setTagFiles( activeTagsFiles );

	config->sync();

	emit newTagsfileName( tagfilePath->url() );
}

void CTags2SettingsWidget::slotAccept( )
{
	storeSettings();
}

void CTags2SettingsWidget::createNewTagSlot()
{
	CreateTagFile* dlg = new CreateTagFile;
	if ( dlg->exec() == QDialog::Accepted )
	{
		m_part->createTagsFile( dlg->tagsfilePath(), dlg->directory() );
		new TagsItem( otherTagFiles, dlg->name(), dlg->tagsfilePath(), true );
	}
}

void CTags2SettingsWidget::addNewTagFile()
{
	SelectTagFile* dlg = new SelectTagFile;

	if ( dlg->exec() == QDialog::Accepted )
	{
		new TagsItem( otherTagFiles, dlg->name(), dlg->tagsfilePath(), true );
	}
}

void CTags2SettingsWidget::removeTagFile()
{
	if (!otherTagFiles->selectedItem())
		return;

	delete otherTagFiles->selectedItem();
}


#include "ctags2_settingswidget.moc"

// kate: space-indent off; indent-width 4; tab-width 4; show-tabs off;
