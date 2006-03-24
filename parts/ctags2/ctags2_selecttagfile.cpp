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

#include "ctags2_selecttagfile.h"


SelectTagFile::SelectTagFile( QWidget* parent, const char* name, bool modal, WFlags fl)
: SelectTagFileBase( parent, name, modal, fl )
{
	tagFile->setMode( KFile::File | KFile::ExistingOnly | KFile::LocalOnly );
	okButton->setEnabled( false );
}

SelectTagFile::~SelectTagFile()
{
}

void SelectTagFile::validate()
{
	bool valid = ( !displayName->text().isEmpty() && !tagFile->url().isEmpty() );

	okButton->setEnabled( valid );
}

QString SelectTagFile::name( )
{
	return displayName->text();
}

QString SelectTagFile::tagsfilePath( )
{
	return tagFile->url();
}


// kate: space-indent off; indent-width 4; tab-width 4; show-tabs off;
