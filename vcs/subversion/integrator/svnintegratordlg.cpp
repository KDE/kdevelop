/***************************************************************************
 *   Copyright (C) 2004 by Alexander Dymo                                  *
 *   adymo@kdevelop.org                                                    *
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
#include "svnintegratordlg.h"
#include <kurl.h>
#include <kio/jobclasses.h>
#include <kio/job.h>
#include <kurlrequester.h>
#include <kdebug.h>
#include <qradiobutton.h>
#include <kio/netaccess.h>
#include <kio/scheduler.h>
#include <kprocess.h>

using namespace KIO;

SvnIntegratorDlg::SvnIntegratorDlg(QWidget *parent, const char *name)
    :SvnIntegratorDlgBase(parent, name)
{
}

void SvnIntegratorDlg::accept()
{
	KURL servURL (repos1->url());
	if ( servURL.isEmpty() ) return;
	if ( ! servURL.protocol().startsWith( "svn" ) )
		servURL.setProtocol( "svn+" + servURL.protocol() ); //make sure it starts with "svn"
	kdDebug() << "servURL : " << servURL.prettyURL() << endl;

	if ( importProject->isChecked() ) {
		QByteArray parms;
		QDataStream s( parms, IO_WriteOnly );
		int cmd = 5;
		s << cmd << servURL << KURL::fromPathOrURL( m_projectLocation );
		SimpleJob * job = KIO::special(servURL, parms, true);
		NetAccess::synchronousRun(job, 0);
	} else if ( createProject->isChecked() ) {
/*		KURL miscURL = servURL;
		miscURL.setPath(servURL.path() + "/tags/");
		KIO::SimpleJob * job = KIO::mkdir(miscURL);
		NetAccess::synchronousRun(job, 0);
		miscURL.setPath(servURL.path() + "/branches/");
		job = KIO::mkdir(miscURL);
		NetAccess::synchronousRun(job, 0);
		miscURL.setPath(servURL.path() + "/vendor/");
		job = KIO::mkdir(miscURL);
		NetAccess::synchronousRun(job, 0);*/
		KURL::List list;
		KURL miscURL = servURL;
		miscURL.setPath( servURL.path() + "/tags/" );
		list << miscURL;
		miscURL.setPath( servURL.path() + "/branches/" );
		list << miscURL;
		miscURL.setPath( servURL.path() + "/vendor/" );
		list << miscURL;

		QByteArray parms;
		QDataStream s( parms, IO_WriteOnly );
		int cmd = 10; // MKDIR(list)
		s << cmd << list;
		SimpleJob *job = KIO::special(servURL, parms, true);
		NetAccess::synchronousRun(job, 0);

		QByteArray parms2;
		QDataStream s2( parms2, IO_WriteOnly );
		cmd = 5; //IMPORT
		servURL.setPath(servURL.path()+ "/trunk/");
		s2 << cmd << servURL << KURL::fromPathOrURL( m_projectLocation );
		job = KIO::special(servURL, parms2, true);
		NetAccess::synchronousRun(job, 0);
	}

	//delete the template directory and checkout a fresh one from the server
    KProcess *rmproc = new KProcess();
    *rmproc << "rm";
    *rmproc << "-f" << "-r" << m_projectLocation;
    rmproc->start(KProcess::Block);
        
	QByteArray parms3;
	QDataStream s3( parms3, IO_WriteOnly );
	int cmd2 = 1; //CHECKOUT
	int rev = -1;
	//servURL should be set correctly above
	s3 << cmd2 << servURL << KURL::fromPathOrURL( m_projectLocation ) << rev << QString( "HEAD" );
	SimpleJob *job2 = KIO::special(servURL, parms3, true);
	NetAccess::synchronousRun(job2, 0);
}

void SvnIntegratorDlg::init(const QString &projectName, const QString &projectLocation)
{
	m_name = projectName;
	m_projectLocation = projectLocation;
}

QWidget *SvnIntegratorDlg::self()
{
    return const_cast<SvnIntegratorDlg*>(this);
}

#include "svnintegratordlg.moc"
