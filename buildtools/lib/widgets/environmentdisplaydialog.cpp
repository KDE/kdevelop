/* This file is part of the KDE project
   Copyright (C) 2007 Jens Dagerbo <jens.dagerbo@swipnet.se>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <unistd.h>
#include <klistview.h>
#include <qstringlist.h>

#include "environmentdisplaydialog.h"

EnvironmentDisplayDialog::EnvironmentDisplayDialog(QWidget* parent, const char* name, bool modal, WFlags fl)
: EnvironmentDisplayDialogBase(parent,name, modal,fl)
{
	QStringList environment;
	char ** e = ::environ;
	
	while ( *e ) 
	{
		environment << *e;
		e++;
	}

	QStringList::ConstIterator it = environment.begin();
	while( it !=environment.end() )
	{
		QStringList pair = QStringList::split( QChar('='), *it );
		if ( pair.count() == 2 )
		{
			new QListViewItem( environmentListView, pair.first(), pair.last() );
		}
		++it;
	}
}


#include "environmentdisplaydialog.moc"

