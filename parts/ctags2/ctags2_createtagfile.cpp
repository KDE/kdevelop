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

#include <kurlrequester.h>
#include <qlineedit.h>

#include "ctags2_createtagfile.h"
#include "ctags2_part.h"


CreateTagFile::CreateTagFile(QWidget* parent, const char* name, bool modal, WFlags fl)
: CTags2CreateTagFileBase( parent, name, modal, fl )
{
	dirToTag->setMode( KFile::Directory | KFile::ExistingOnly | KFile::LocalOnly);
	newTagFilePath->setMode( KFile::File | KFile::LocalOnly );
	createTagFileButton->setEnabled( false );
}

CreateTagFile::~CreateTagFile()
{
}

void CreateTagFile::validate()
{
	bool valid = ( !displayName->text().isEmpty() && !newTagFilePath->url().isEmpty() && !dirToTag->url().isEmpty() );

	createTagFileButton->setEnabled( valid );
}

QString CreateTagFile::name( )
{
	return displayName->text();
}

QString CreateTagFile::tagsfilePath( )
{
	return newTagFilePath->url();
}

QString CreateTagFile::directory( )
{
	return dirToTag->url();
}

// kate: space-indent off; indent-width 4; tab-width 4; show-tabs off;
