/* 
    This file is part of the KDE libraries
    Copyright (C) 1997 Torben Weis (weis@kde.org)
    Copyright (C) 1998 Matthias Ettrich (ettrich@kde.org)
    Copyright (C) 1999 David Faure (faure@kde.org)

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

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

#include <cstring>

#include <qstring.h>
#include <qapplication.h>
#include <qfile.h>
#include <qmetaobject.h>

#include <kapplication.h>
#include <klocale.h>
#include <ktempfile.h>
#include <kdebug.h>
#include <kurl.h>
#include <kio/job.h>
#include <kio/scheduler.h>

#include "netaccess.h"

using namespace KIO_COMPAT;

QString * NetAccess::lastErrorMsg;
int NetAccess::lastErrorCode = 0;
QStringList* NetAccess::tmpfiles;

bool NetAccess::download(const KURL& u, QString & target)
{
  return NetAccess::download (u, target, 0);
}

bool NetAccess::download(const KURL& u, QString & target, QWidget* window)
{
  if (u.isLocalFile()) {
    // file protocol. We do not need the network
    target = u.path();
    bool accessible = checkAccess(target, R_OK);
    if(!accessible)
    {
        if(!lastErrorMsg)
            lastErrorMsg = new QString;
        *lastErrorMsg = i18n("File '%1' is not readable").arg(target);
        lastErrorCode = KIO::ERR_COULD_NOT_READ;
    }
    return accessible;
  }

  if (target.isEmpty())
  {
      KTempFile tmpFile;
      target = tmpFile.name();
      if (!tmpfiles)
          tmpfiles = new QStringList;
      tmpfiles->append(target);
  }

  NetAccess kioNet;
  KURL dest;
  dest.setPath( target );
  return kioNet.filecopyInternal( u, dest, -1, true /*overwrite*/,
                                  false, window, false /*copy*/);
}

bool NetAccess::upload(const QString& src, const KURL& target)
{
  return NetAccess::upload(src, target, 0);
}

bool NetAccess::upload(const QString& src, const KURL& target, QWidget* window)
{
  if (target.isEmpty())
    return false;

  // If target is local... well, just copy. This can be useful
  // when the client code uses a temp file no matter what.
  // Let's make sure it's not the exact same file though
  if (target.isLocalFile() && target.path() == src)
    return true;

  NetAccess kioNet;
  KURL s;
  s.setPath(src);
  return kioNet.filecopyInternal( s, target, -1, true /*overwrite*/,
                                  false, window, false /*copy*/ );
}

bool NetAccess::copy( const KURL & src, const KURL & target )
{
  return NetAccess::file_copy( src, target, -1, false /*not overwrite*/, false, 0L );
}

bool NetAccess::copy( const KURL & src, const KURL & target, QWidget* window )
{
  return NetAccess::file_copy( src, target, -1, false /*not overwrite*/, false, window );
}

bool NetAccess::file_copy( const KURL& src, const KURL& target, int permissions,
                           bool overwrite, bool resume, QWidget* window )
{
  NetAccess kioNet;
  return kioNet.filecopyInternal( src, target, permissions, overwrite, resume,
                                  window, false /*copy*/ );
}


bool NetAccess::file_move( const KURL& src, const KURL& target, int permissions,
                           bool overwrite, bool resume, QWidget* window )
{
  NetAccess kioNet;
  return kioNet.filecopyInternal( src, target, permissions, overwrite, resume,
                                  window, true /*move*/ );
}

bool NetAccess::dircopy( const KURL & src, const KURL & target )
{
  return NetAccess::dircopy( src, target, 0 );
}

bool NetAccess::dircopy( const KURL & src, const KURL & target, QWidget* window )
{
  KURL::List srcList;
  srcList.append( src );
  return NetAccess::dircopy( srcList, target, window );
}

bool NetAccess::dircopy( const KURL::List & srcList, const KURL & target, QWidget* window )
{
  NetAccess kioNet;
  return kioNet.dircopyInternal( srcList, target, window, false /*copy*/ );
}

bool NetAccess::move( const KURL& src, const KURL& target, QWidget* window )
{
  KURL::List srcList;
  srcList.append( src );
  return NetAccess::move( srcList, target, window );
}

bool NetAccess::move( const KURL::List& srcList, const KURL& target, QWidget* window )
{
  NetAccess kioNet;
  return kioNet.dircopyInternal( srcList, target, window, true /*move*/ );
}

bool NetAccess::exists( const KURL & url )
{
  return NetAccess::exists( url, false, 0 );
}

bool NetAccess::exists( const KURL & url, QWidget* window )
{
  return NetAccess::exists( url, false, window );
}

bool NetAccess::exists( const KURL & url, bool source )
{
  return NetAccess::exists( url, source, 0 );
}

bool NetAccess::exists( const KURL & url, bool source, QWidget* window )
{
  if ( url.isLocalFile() )
    return QFile::exists( url.path() );
  NetAccess kioNet;
  return kioNet.statInternal( url, 0 /*no details*/, source, window );
}

bool NetAccess::stat( const KURL & url, KIO::UDSEntry & entry )
{
  return NetAccess::stat( url, entry, 0 );
}

bool NetAccess::stat( const KURL & url, KIO::UDSEntry & entry, QWidget* window )
{
  NetAccess kioNet;
  bool ret = kioNet.statInternal( url, 2 /*all details*/, true /*source*/, window );
  if (ret)
    entry = kioNet.m_entry;
  return ret;
}

bool NetAccess::del( const KURL & url )
{
  return NetAccess::del( url, 0 );
}

bool NetAccess::del( const KURL & url, QWidget* window )
{
  NetAccess kioNet;
  return kioNet.delInternal( url, window );
}

bool NetAccess::mkdir( const KURL & url, int permissions )
{
  return NetAccess::mkdir( url, 0, permissions );
}

bool NetAccess::mkdir( const KURL & url, QWidget* window, int permissions )
{
  NetAccess kioNet;
  return kioNet.mkdirInternal( url, permissions, window );
}

QString NetAccess::fish_execute( const KURL & url, const QString command, QWidget* window )
{
  NetAccess kioNet;
  return kioNet.fish_executeInternal( url, command, window );
}

bool NetAccess::synchronousRun( KIO::Job* job, QWidget* window, QByteArray* data,
                                KURL* finalURL, QMap<QString, QString>* metaData )
{
  NetAccess kioNet;
  return kioNet.synchronousRunInternal( job, window, data, finalURL, metaData );
}

QString NetAccess::mimetype( const KURL& url )
{
  NetAccess kioNet;
  return kioNet.mimetypeInternal( url, 0 );
}

QString NetAccess::mimetype( const KURL& url, QWidget* window )
{
  NetAccess kioNet;
  return kioNet.mimetypeInternal( url, window );
}

void NetAccess::removeTempFile(const QString& name)
{
  if (!tmpfiles)
    return;
  if (tmpfiles->contains(name))
  {
    unlink(QFile::encodeName(name));
    tmpfiles->remove(name);
  }
}

bool NetAccess::filecopyInternal(const KURL& src, const KURL& target, int permissions,
                                 bool overwrite, bool resume, QWidget* window, bool move)
{
  bJobOK = true; // success unless further error occurs

  KIO::Scheduler::checkSlaveOnHold(true);
  KIO::Job * job = move
                   ? KIO::file_move( src, target, permissions, overwrite, resume )
                   : KIO::file_copy( src, target, permissions, overwrite, resume );
  job->setWindow (window);
  connect( job, SIGNAL( result (KIO::Job *) ),
           this, SLOT( slotResult (KIO::Job *) ) );

  enter_loop();
  return bJobOK;
}

bool NetAccess::dircopyInternal(const KURL::List& src, const KURL& target,
                                QWidget* window, bool move)
{
  bJobOK = true; // success unless further error occurs

  KIO::Job * job = move
                   ? KIO::move( src, target )
                   : KIO::copy( src, target );
  job->setWindow (window);
  connect( job, SIGNAL( result (KIO::Job *) ),
           this, SLOT( slotResult (KIO::Job *) ) );

  enter_loop();
  return bJobOK;
}

bool NetAccess::statInternal( const KURL & url, int details, bool source,
                              QWidget* window )
{
  bJobOK = true; // success unless further error occurs
  KIO::StatJob * job = KIO::stat( url, !url.isLocalFile() );
  job->setWindow (window);
  job->setDetails( details );
  job->setSide( source );
  connect( job, SIGNAL( result (KIO::Job *) ),
           this, SLOT( slotResult (KIO::Job *) ) );
  enter_loop();
  return bJobOK;
}

bool NetAccess::delInternal( const KURL & url, QWidget* window )
{
  bJobOK = true; // success unless further error occurs
  KIO::Job * job = KIO::del( url );
  job->setWindow (window);
  connect( job, SIGNAL( result (KIO::Job *) ),
           this, SLOT( slotResult (KIO::Job *) ) );
  enter_loop();
  return bJobOK;
}

bool NetAccess::mkdirInternal( const KURL & url, int permissions,
                               QWidget* window )
{
  bJobOK = true; // success unless further error occurs
  KIO::Job * job = KIO::mkdir( url, permissions );
  job->setWindow (window);
  connect( job, SIGNAL( result (KIO::Job *) ),
           this, SLOT( slotResult (KIO::Job *) ) );
  enter_loop();
  return bJobOK;
}

QString NetAccess::mimetypeInternal( const KURL & url, QWidget* window )
{
  bJobOK = true; // success unless further error occurs
  m_mimetype = QString::fromLatin1("unknown");
  KIO::Job * job = KIO::mimetype( url );
  job->setWindow (window);
  connect( job, SIGNAL( result (KIO::Job *) ),
           this, SLOT( slotResult (KIO::Job *) ) );
  connect( job, SIGNAL( mimetype (KIO::Job *, const QString &) ),
           this, SLOT( slotMimetype (KIO::Job *, const QString &) ) );
  enter_loop();
  return m_mimetype;
}

void NetAccess::slotMimetype( KIO::Job *, const QString & type  )
{
  m_mimetype = type;
}

QString NetAccess::fish_executeInternal(const KURL & url, const QString command, QWidget* window)
{
  QString target, remoteTempFileName, resultData;
  KURL tempPathUrl;
  KTempFile tmpFile;
  tmpFile.setAutoDelete( true );

  if( url.protocol() == "fish" )
  {
    // construct remote temp filename
    tempPathUrl = url;
    remoteTempFileName = tmpFile.name();
    // only need the filename KTempFile adds some KDE specific dirs
    // that probably does not exist on the remote side
    int pos = remoteTempFileName.findRev('/');
    remoteTempFileName = "/tmp/fishexec_" + remoteTempFileName.mid(pos + 1);
    tempPathUrl.setPath( remoteTempFileName );
    bJobOK = true; // success unless further error occurs
    QByteArray packedArgs;
    QDataStream stream( packedArgs, IO_WriteOnly );

    stream << int('X') << tempPathUrl << command;

    KIO::Job * job = KIO::special( tempPathUrl, packedArgs, true );
    job->setWindow( window );
    connect( job, SIGNAL( result (KIO::Job *) ),
             this, SLOT( slotResult (KIO::Job *) ) );
    enter_loop();

    // since the KIO::special does not provide feedback we need to download the result
    if( NetAccess::download( tempPathUrl, target, window ) )
    {
      QFile resultFile( target );

      if (resultFile.open( IO_ReadOnly ))
      {
        QTextStream ts( &resultFile );
        ts.setEncoding( QTextStream::Locale ); // Locale??
        resultData = ts.read();
        resultFile.close();
        NetAccess::del( tempPathUrl, window );
      }
    }
  }
  else
  {
    resultData = QString( "ERROR: Unknown protocol '%1'" ).arg( url.protocol() );
  }
  return resultData;
}

bool NetAccess::synchronousRunInternal( KIO::Job* job, QWidget* window, QByteArray* data,
                                        KURL* finalURL, QMap<QString,QString>* metaData )
{
  job->setWindow( window );

  m_metaData = metaData;
  if ( m_metaData ) {
      for ( QMap<QString, QString>::iterator it = m_metaData->begin(); it != m_metaData->end(); ++it ) {
          job->addMetaData( it.key(), it.data() );
      }
  }

  if ( finalURL ) {
      KIO::SimpleJob *sj = dynamic_cast<KIO::SimpleJob*>( job );
      if ( sj ) {
          m_url = sj->url();
      }
  }

  connect( job, SIGNAL( result (KIO::Job *) ),
           this, SLOT( slotResult (KIO::Job *) ) );

  QMetaObject *meta = job->metaObject();

  static const char dataSignal[] = "data(KIO::Job*,const QByteArray&)";
  if ( meta->findSignal( dataSignal ) != -1 ) {
      connect( job, SIGNAL(data(KIO::Job*,const QByteArray&)),
               this, SLOT(slotData(KIO::Job*,const QByteArray&)) );
  }

  static const char redirSignal[] = "redirection(KIO::Job*,const KURL&)";
  if ( meta->findSignal( redirSignal ) != -1 ) {
      connect( job, SIGNAL(redirection(KIO::Job*,const KURL&)),
               this, SLOT(slotRedirection(KIO::Job*, const KURL&)) );
  }

  enter_loop();

  if ( finalURL )
      *finalURL = m_url;
  if ( data )
      *data = m_data;

  return bJobOK;
}

// If a troll sees this, he kills me
void qt_enter_modal( QWidget *widget );
void qt_leave_modal( QWidget *widget );

void NetAccess::enter_loop()
{
  QWidget dummy(0,0,WType_Dialog | WShowModal);
  dummy.setFocusPolicy( QWidget::NoFocus );
  qt_enter_modal(&dummy);
  qApp->enter_loop();
  qt_leave_modal(&dummy);
}

void NetAccess::slotResult( KIO::Job * job )
{
  lastErrorCode = job->error();
  bJobOK = !job->error();
  if ( !bJobOK )
  {
    if ( !lastErrorMsg )
      lastErrorMsg = new QString;
    *lastErrorMsg = job->errorString();
  }
  if ( job->isA("KIO::StatJob") )
    m_entry = static_cast<KIO::StatJob *>(job)->statResult();

  if ( m_metaData )
    *m_metaData = job->metaData();

  qApp->exit_loop();
}

void NetAccess::slotData( KIO::Job*, const QByteArray& data )
{
  if ( data.isEmpty() )
    return;

  unsigned offset = m_data.size();
  m_data.resize( offset + data.size() );
  std::memcpy( m_data.data() + offset, data.data(), data.size() );
}

void NetAccess::slotRedirection( KIO::Job*, const KURL& url )
{
  m_url = url;
}

#include "netaccess.moc"
