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

#include <qwhatsthis.h>
#include <qlistbox.h>
#include <qpopupmenu.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qradiobutton.h>

#include <kiconloader.h>
#include <klocale.h>
#include <klineedit.h>
#include <kio/netaccess.h>

#include <kdevgenericfactory.h>
#include <kdevcore.h>
#include <kdevmainwindow.h>
#include <kdevproject.h>


#include "copyto_part.h"
#include "copytodialog.h"

static const KAboutData data("kdevcopyto", I18N_NOOP("Copy To..."), "1.0");

typedef KDevGenericFactory<CopyToPart> CopyToFactory;
K_EXPORT_COMPONENT_FACTORY( libkdevcopyto, CopyToFactory( &data ) );

CopyToPart::CopyToPart(QObject *parent, const char *name, const QStringList& )
  : KDevPlugin("copyto", "copyto", parent, name ? name : "CopyToPart" )
{
	setInstance(CopyToFactory::instance());
//	setXMLFile("kdevpart_copyto.rc");

	connect( core(), SIGNAL(contextMenu(QPopupMenu*, const Context* )), this, SLOT(contextMenu(QPopupMenu*, const Context* )) );  
}


CopyToPart::~CopyToPart()
{
}

void CopyToPart::contextMenu( QPopupMenu * popup, const Context * context )
{
	if ( context->hasType( Context::FileContext ) )
	{
		popup->insertItem( i18n( "Copy To..." ), this, SLOT(doCopy()) );
		
		const FileContext * fContext = static_cast<const FileContext*>( context );
		_fileList = fContext->urls();
	}
	
}

void CopyToPart::doCopy( )
{
	QStringList files;
	
	KURL::List::Iterator it = _fileList.begin();
	while( it != _fileList.end() )
	{
		QString file = relativeProjectPath( (*it).path() );
		if ( !file.isEmpty() )
		{
			files << file;
		}
		++it;
	}
	
	if ( files.isEmpty() ) return;
	
	CopyToDialog dlg;
	dlg.fileList->insertStringList( files );
	
	if ( dlg.exec() == QDialog::Rejected ) return;
	
	KURL desturl = KURL::fromPathOrURL( dlg.url_line->text() );
	
	if ( !desturl.isValid() || !KIO::NetAccess::exists( desturl, true, 0 ) ) return;
	
	if ( dlg.traditional->isOn() )
	{
		it = _fileList.begin();
		while( it != _fileList.end() )
		{
			KURL dest = KURL::fromPathOrURL( desturl.url( +1 ) + (*it).fileName() );
			KIO::NetAccess::upload( (*it).path(), dest, 0 );
			++it;
		}
	}
	else
	{
		QStringList::Iterator it = files.begin();
		while( it != files.end() )
		{
			KURL dest = KURL::fromPathOrURL( desturl.url( +1 ) + *it );
			KIO::NetAccess::upload( project()->projectDirectory() + "/" + *it, dest, 0 );
			++it;
		}
	}
	
}

QString CopyToPart::relativeProjectPath( QString path )
{
	QString projectpath = project()->projectDirectory() + "/";
	if ( path.left( projectpath.length() ) == projectpath )
	{
		path = path.mid( projectpath.length() );
		return path;
	}
	return QString::null;
}

#include "copyto_part.moc"
