/*
 *  Copyright (C) 2004 Ahn, Duk J.(adjj22@kornet.net) (adjj1@hanmail.net)
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
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 *
 */

#include <klistbox.h>
#include <klocale.h>
#include <kurl.h>
#include <kdebug.h>
#include <ksqueezedtextlabel.h>

#include <codemodel.h>

#include "quickopenfunctionchooseform.h"

QuickOpenFunctionChooseForm::QuickOpenFunctionChooseForm(QWidget* parent, const char* name, bool modal, WFlags fl)
: QuickOpenFunctionChooseFormBase(parent,name, modal,fl)
{
	setCaption( i18n("Select One Argument or File of Function %1").arg( name ) );
	slotArgsChange( 0 );
	slotFileChange( 0 );
}

QuickOpenFunctionChooseForm::~QuickOpenFunctionChooseForm()
{
}

void QuickOpenFunctionChooseForm::slotArgsChange( int id )
{
	argBox->setCurrentItem( id );
}

void QuickOpenFunctionChooseForm::slotFileChange( int id )
{
	fileBox->setCurrentItem( id );
	filepathlabel->setText( m_relPaths[id] );
}


#include "quickopenfunctionchooseform.moc"

