/***************************************************************************
 *   Copyright (C) 2004 by Jens Dagerbo                                    *
 *   jens.dagerbo@swipnet.se                                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <qpushbutton.h>

#include <kfiledialog.h>
#include <kurl.h>
#include <klineedit.h>
#include <kurlcompletion.h> 

#include "copytodialog.h"

CopyToDialog::CopyToDialog(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
: CopyToDialogBase(parent,name, modal,fl)
{
	_urlCompletion = new KURLCompletion( KURLCompletion::DirCompletion );
	url_line->setCompletionObject( _urlCompletion );
		
	connect( url_button, SIGNAL(clicked()), this, SLOT(openURLDialog()) );
}

CopyToDialog::~CopyToDialog()
{
	delete _urlCompletion;
}

void CopyToDialog::openURLDialog( )
{
	KURL desturl = KFileDialog::getExistingURL( url_line->text() );
	url_line->setText( desturl.url( +1 ) );
}

#include "copytodialog.moc"

