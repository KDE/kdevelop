/* Copyright (C) 2003
	 Mickael Marchand <marchand@kde.org>

	 This program is free software; you can redistribute it and/or
	 modify it under the terms of the GNU General Public
	 License as published by the Free Software Foundation; either
	 version 2 of the License, or (at your option) any later version.

	 This program is distributed in the hope that it will be useful,
	 but WITHOUT ANY WARRANTY; without even the implied warranty of
	 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	 General Public License for more details.

	 You should have received a copy of the GNU General Public License
	 along with this program; see the file COPYING.  If not, write to
	 the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
	 Boston, MA 02111-1307, USA.
	 */

#include <kparts/part.h>
#include <kdevcore.h>

#include "subversion_part.h"
#include "subversion_core.h"
#include "subversion_widget.h"
#include <kdevmainwindow.h>
#include "svn_co.h"
#include <kurlrequester.h>
#include <klineedit.h>
#include <kio/job.h>
#include <kdebug.h>
#include <kmainwindow.h>

using namespace KIO;

subversionCore::subversionCore(subversionPart *part)
	: QObject(this, "subversion core") {
		m_part = part;
		m_widget = new subversionWidget(part, 0 , "subversionprocesswidget");
	}

subversionCore::~subversionCore() {
	if ( processWidget() ) {
		m_part->mainWindow()->removeView( m_widget );
		delete m_widget;
	}
}

subversionWidget *subversionCore::processWidget() const {
	return m_widget;
}

void subversionCore::update( const KURL::List& list ) {
	KURL servURL = m_part->baseURL();
	if ( servURL.isEmpty() ) servURL="svn+http://blah/";
	if ( ! servURL.protocol().startsWith( "svn" ) ) {
		servURL.setProtocol( "svn+" + servURL.protocol() ); //make sure it starts with "svn"
	}
	kdDebug() << "servURL : " << servURL.prettyURL() << endl;
	for ( QValueListConstIterator<KURL> it = list.begin(); it != list.end() ; ++it ) {
		kdDebug() << "updating : " << (*it).prettyURL() << endl;
		QByteArray parms;
		QDataStream s( parms, IO_WriteOnly );
		int cmd = 2;
		int rev = -1;
		s << cmd << *it << rev << QString( "HEAD" );
		SimpleJob * job = KIO::special(servURL, parms, true);
		job->setWindow( m_part->mainWindow()->main() );
		connect( job, SIGNAL( result( KIO::Job * ) ), this, SLOT( slotResult( KIO::Job * ) ) );
	}
}

void subversionCore::commit( const KURL::List& list ) {
	KURL servURL = m_part->baseURL();
	if ( servURL.isEmpty() ) servURL="svn+http://blah/";
	if ( ! servURL.protocol().startsWith( "svn" ) ) {
		servURL.setProtocol( "svn+" + servURL.protocol() ); //make sure it starts with "svn"
	}
	kdDebug() << "servURL : " << servURL.prettyURL() << endl;
	for ( QValueListConstIterator<KURL> it = list.begin(); it != list.end() ; ++it ) {
		kdDebug() << "commiting : " << (*it).prettyURL() << endl;
		QByteArray parms;
		QDataStream s( parms, IO_WriteOnly );
		int cmd = 3;
		s << cmd << *it;
		SimpleJob * job = KIO::special(servURL, parms, true);
		job->setWindow( m_part->mainWindow()->main() );
		connect( job, SIGNAL( result( KIO::Job * ) ), this, SLOT( slotResult( KIO::Job * ) ) );
	}
}

void subversionCore::add( const KURL::List& list ) {
	KURL servURL = m_part->baseURL();
	if ( servURL.isEmpty() ) servURL="svn+http://blah/";
	if ( ! servURL.protocol().startsWith( "svn" ) ) {
		servURL.setProtocol( "svn+" + servURL.protocol() ); //make sure it starts with "svn"
	}
	kdDebug() << "servURL : " << servURL.prettyURL() << endl;
	for ( QValueListConstIterator<KURL> it = list.begin(); it != list.end() ; ++it ) {
		kdDebug() << "adding : " << (*it).prettyURL() << endl;
		QByteArray parms;
		QDataStream s( parms, IO_WriteOnly );
		int cmd = 6;
		s << cmd << *it;
		SimpleJob * job = KIO::special(servURL, parms, true);
		job->setWindow( m_part->mainWindow()->main() );
		connect( job, SIGNAL( result( KIO::Job * ) ), this, SLOT( slotResult( KIO::Job * ) ) );
	}
}

void subversionCore::del( const KURL::List& list ) {
	KURL servURL = m_part->baseURL();
	if ( servURL.isEmpty() ) servURL="svn+http://blah/";
	if ( ! servURL.protocol().startsWith( "svn" ) ) {
		servURL.setProtocol( "svn+" + servURL.protocol() ); //make sure it starts with "svn"
	}
	kdDebug() << "servURL : " << servURL.prettyURL() << endl;
	for ( QValueListConstIterator<KURL> it = list.begin(); it != list.end() ; ++it ) {
		kdDebug() << "deleting : " << (*it).prettyURL() << endl;
		QByteArray parms;
		QDataStream s( parms, IO_WriteOnly );
		int cmd = 7;
		s << cmd << *it;
		SimpleJob * job = KIO::special(servURL, parms, true);
		job->setWindow( m_part->mainWindow()->main() );
		connect( job, SIGNAL( result( KIO::Job * ) ), this, SLOT( slotResult( KIO::Job * ) ) );
	}
}

void subversionCore::checkout() {
	svn_co checkoutDlg;

	if ( checkoutDlg.exec() == QDialog::Accepted ) {
		//checkout :)
		QByteArray parms;
		QDataStream s( parms, IO_WriteOnly );
		KURL servURL ( checkoutDlg.serverURL->url() );
		wcPath = checkoutDlg.localDir->url() + "/" + checkoutDlg.newDir->text();
		int cmd = 1;
		int rev = -1;
		s << cmd << servURL << KURL( wcPath ) << rev << QString( "HEAD" );
		servURL.setProtocol( "svn+" + servURL.protocol() ); //make sure it starts with "svn"
		SimpleJob * job = KIO::special(servURL,parms, true);
		job->setWindow( m_part->mainWindow()->main() );
		connect( job, SIGNAL( result( KIO::Job * ) ), this, SLOT( slotEndCheckout( KIO::Job * ) ) );
	}
}

void subversionCore::slotEndCheckout( KIO::Job * job ) {
		if ( job->error() ) {
			job->showErrorDialog( m_part->mainWindow()->main() );
			emit checkoutFinished( QString::null );
		} else 
			emit checkoutFinished(wcPath);
}

void subversionCore::slotResult( KIO::Job * job ) {
		if ( job->error() )
			job->showErrorDialog( m_part->mainWindow()->main() );
}

void subversionCore::createNewProject( const QString& dirName, const KURL& importURL, bool init ) {

}

#include "subversion_core.moc"
