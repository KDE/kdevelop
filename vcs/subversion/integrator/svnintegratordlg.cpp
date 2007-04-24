/***************************************************************************
 *   Copyright (C) 2004 by Alexander Dymo                                  *
 *   adymo@kdevelop.org                                                    *
 *   Copyright (C) 2004                                                    *
 *   Mickael Marchand <marchand@kde.org>                                   *
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
#include "blockingkprocess.h"
#include <kurl.h>
#include <kio/jobclasses.h>
#include <kio/job.h>
#include <kurlrequester.h>
#include <kdebug.h>
#include <qradiobutton.h>
#include <kio/scheduler.h>
#include <kprocess.h>
#include <kdeversion.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kdebug.h>

#include <kio/netaccess.h>

using namespace KIO;

SvnIntegratorDlg::SvnIntegratorDlg( QWidget *parent, const char *name )
        : SvnIntegratorDlgBase( parent, name )
{
    repos1->setMode( KFile::Directory );
}

void SvnIntegratorDlg::accept()
{
    // to let ioslave know which protocol it should start.
    KURL protocolUrl = KURL("kdevsvn+svn://blah/");
    KURL servURL( repos1->url() );
    if ( servURL.isEmpty() ) return;
    
    kdDebug( 9036 ) << "servURL : " << servURL.prettyURL() << endl;
    if ( createProject->isChecked() )
    {
        KURL::List list;
        list << servURL; // project root directory
        KURL miscURL = servURL.url();
        miscURL.setPath( servURL.path() + "/tags/" );
        list << miscURL;
        miscURL.setPath( servURL.path() + "/branches/" );
        list << miscURL;
        miscURL.setPath( servURL.path() + "/trunk/" );
        list << miscURL;

        QByteArray parms;
        QDataStream s( parms, IO_WriteOnly );
        int cmd = 10; // MKDIR(list)
        s << cmd << list;
        KIO::SimpleJob* job = KIO::special( protocolUrl, parms, true );
        if( !NetAccess::synchronousRun( job, 0 ) ){
            KMessageBox::error( this, i18n("Fail to create project directories on repository") );
            return;
        }

        QByteArray parms2;
        QDataStream s2( parms2, IO_WriteOnly );
        cmd = 5; //IMPORT
        servURL.setPath( servURL.path() + "/trunk/" );
        s2 << cmd << servURL << KURL::fromPathOrURL( m_projectLocation );
        KIO::SimpleJob* importJob = KIO::special( protocolUrl, parms2, true );
        if( !NetAccess::synchronousRun( importJob, 0 ) ){
            KMessageBox::error( this, i18n("Fail to import into repository.") );
            return;
        }
    }
    //delete the template directory and checkout a fresh one from the server
    BlockingKProcess *rmproc = new BlockingKProcess();
    *rmproc << "rm";
    *rmproc << "-f" << "-r" << m_projectLocation;
    rmproc->start();
    
    delete rmproc;
    rmproc = NULL;

    QByteArray parms3;
    QDataStream s3( parms3, IO_WriteOnly );
    int cmd2 = 1; //CHECKOUT
    int rev = -1;
    
    s3 << cmd2 << servURL << KURL::fromPathOrURL( m_projectLocation ) << rev << QString( "HEAD" );
    SimpleJob *job2 = KIO::special( protocolUrl, parms3, true );
    if( ! NetAccess::synchronousRun( job2, 0 ) ){
        // Checkout failed
        KMessageBox::error(this, i18n("Fail to checkout from repository.") );
        return;
    }
}

void SvnIntegratorDlg::init( const QString &projectName, const QString &projectLocation )
{
    m_name = projectName;
    m_projectLocation = projectLocation;
}

QWidget *SvnIntegratorDlg::self()
{
    return const_cast<SvnIntegratorDlg*>( this );
}

#include "svnintegratordlg.moc"
