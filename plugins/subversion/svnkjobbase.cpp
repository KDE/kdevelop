/***************************************************************************
 *   Copyright (C) 2007 by Dukju Ahn                                       *
 *   dukjuahn@gmail.com                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "svnkjobbase.h"
#include "subversion_core.h"
#include "subversionthreads.h"
#include "svn_models.h"
#include "vcshelpers.h"
#include "vcsevent.h"
#include "vcsrevision.h"
#include "vcsannotation.h"
#include "vcsdiff.h"
#include <klocale.h>
#include <QVariant>
#include <QMap>
#include <QFile>

class SvnKJobBase::Private
{
public:
    SubversionThread *m_th;
    int m_type;
    QVariant m_variant;
    bool m_validResult;

    // returns QVariant ( const QMap< QString, QVariant > ),
    // where innermost QVariant (which is template argument) is constructed with
    // QVariant( KDevelop::VcsState )
    QVariant result_status()
    {
        QMap< QString, QVariant > retMap;
        SvnStatusJob *thread = dynamic_cast<SvnStatusJob*>(m_th);
        QList<SvnStatusHolder> holderList = thread->m_holderMap.values();

        foreach( SvnStatusHolder _holder, holderList ){

            int stat = KDevelop::Unknown;
            if( _holder.textStatus == svn_wc_status_normal||
                _holder.propStatus == svn_wc_status_normal){

                stat = KDevelop::ItemUpToDate;
            }
            else if( _holder.textStatus == svn_wc_status_added ){
                stat = KDevelop::ItemAdded;
            }
            else if( _holder.textStatus == svn_wc_status_modified ||
                     _holder.propStatus == svn_wc_status_modified ){
                stat = KDevelop::ItemModified;
            }
            else if( _holder.textStatus == svn_wc_status_deleted ){
                stat = KDevelop::ItemDeleted;
            }
            else if( _holder.textStatus == svn_wc_status_conflicted ||
                     _holder.propStatus == svn_wc_status_conflicted ){
                stat = KDevelop::ItemHasConflicts;
            }

            QVariant statVar( stat );
            retMap.insert( _holder.wcPath , statVar );
        }

        QVariant variant( retMap );
        return variant;
    }

    // returns QVariant ( const QList<QVariant> & val ),
    // where innermost QVariant (which is template argument) is constructed with
    // qVariantFromValue( VcsEvent )
    QVariant result_log()
    {
        QList< QVariant > eventList;
        SvnLogviewJob *thread = dynamic_cast<SvnLogviewJob*>(m_th);
        QList<SvnLogHolder> logList = thread->m_loglist;

        foreach( SvnLogHolder _holder, logList ){
            KDevelop::VcsEvent oneEvent;

            KDevelop::VcsRevision revision;
            revision.setRevisionValue( QString::number(_holder.rev), KDevelop::VcsRevision::GlobalNumber );
            oneEvent.setRevision( revision );

            oneEvent.setAuthor( _holder.author );
            oneEvent.setDate( QDateTime::fromString(_holder.date, Qt::ISODate) );
            oneEvent.setMessage( _holder.logmsg );
            // TODO setActions, setItems.

            QVariant eventVariant = qVariantFromValue( oneEvent );
            eventList.append( eventVariant );
        }

        QVariant retVariant( eventList );
        return retVariant;
    }

    // returns QVariant( VcsAnnotation ) which is constructed with qVariantFromValue( VcsAnnotation )
    QVariant result_annotate()
    {
        QList< QVariant > annList;
        SvnBlameJob *thread = dynamic_cast<SvnBlameJob*>(m_th);
        KDevelop::VcsAnnotation vcsAnn;

        QStringList lines;
        QStringList authors;
        QList<QDateTime> dates;
        QList<KDevelop::VcsRevision> revisions;

        foreach( SvnBlameHolder _holder, thread->m_blameList ){
            lines.append( _holder.contents );
            authors.append( _holder.author );
            dates.append( QDateTime::fromString( _holder.date, Qt::ISODate ) );

            KDevelop::VcsRevision rev;
            rev.setRevisionValue( QString::number( _holder.revNo ), KDevelop::VcsRevision::GlobalNumber );
            revisions.append( rev );
        }

        vcsAnn.setLines( lines );
        vcsAnn.setAuthors( authors );
        vcsAnn.setDates( dates );
        vcsAnn.setRevisions( revisions );
        vcsAnn.setLocation( KUrl() ); // TODO modify subversion plugin internal.

        return qVariantFromValue( vcsAnn );
    }

    QVariant result_diff()
    {
        KDevelop::VcsDiff vcsDiff;
        QString allOutputs;

        SvnDiffJob *thread = dynamic_cast<SvnDiffJob*>(m_th);
        QFile file( thread->out_name );
        if( !file.exists() ){
            return QVariant();
        }

        if( file.open( QIODevice::ReadOnly ) ){
            QTextStream stream( &file ); // TODO QDataStream?
            allOutputs = stream.readAll();
            file.close();
        }

        vcsDiff.setDiff( allOutputs );
        // TODO subversion api doesn't let us know the kind of diff output.
//         vcsDiff.setType();
//         vcsDiff.setContentType();
        return qVariantFromValue( vcsDiff );
    }
};

SvnKJobBase::SvnKJobBase( int type, QObject *parent )
    : VcsJob( parent ), d( new Private )
{
    d->m_th = 0;
    d->m_type = type;
    d->m_validResult = false;

    // The forceful termination of thread causes deadlock in some cases.
    // Don't set Killable for a moment
    // setCapabilities( KJob::Killable );
}

SvnKJobBase::~SvnKJobBase()
{
    if( d->m_th ){
        bool ret = d->m_th->requestTerminate(500);
        if( !ret )
            kWarning() << " SvnKJobBase::Destructor: deleting still-running thread " << endl;
        delete d->m_th;
    }
    delete d;
    kDebug() << " SvnKJobBase::Destructor: end cleanup " << endl;
}

KDevelop::VcsJob::JobStatus SvnKJobBase::status()
{
    if( !error() ){
        return KDevelop::VcsJob::JobSucceeded;
    }
    else{
        return KDevelop::VcsJob::JobFailed;
    }
}

void SvnKJobBase::setResult( const QVariant &result )
{
    d->m_variant = result;
    d->m_validResult = true;
}

QVariant SvnKJobBase::fetchResults()
{
//     return d->m_variant;
    switch( type() ){
        case KDevelop::VcsJob::Status:
            return d->result_status();
        case KDevelop::VcsJob::Log:
            return d->result_log();
        case KDevelop::VcsJob::Diff:
            return d->result_diff();
        case KDevelop::VcsJob::Annotate:
            return d->result_annotate();
        default:
            return QVariant();
    }
}

void SvnKJobBase::setSvnThread( SubversionThread *th )
{
    d->m_th = th;
    connect( d->m_th, SIGNAL(finished()), this, SLOT(threadFinished()) );
}

SubversionThread* SvnKJobBase::svnThread()
{
    return d->m_th;
}

KDevelop::VcsJob::JobType SvnKJobBase::type()
{
    return (KDevelop::VcsJob::JobType)(d->m_type);
}

QString SvnKJobBase::smartError()
{
    QString msg = errorText();
    if( msg.isEmpty() ){
        msg = i18n("An error occurred during subversion operation");
    }
    return msg;
}

void SvnKJobBase::start()
{
    if( d->m_validResult ){
        threadFinished();
        return;
    }

    if( !d->m_th ) return;
    d->m_th->start();
}

// KDevelop::VcsJob::JobStatus SvnKJobBase::exec()
// {
//     bool ret = KJob::exec();
//     if( ret ){
//         return KDevelop::VcsJob::JobSucceeded;
//     }
//     else{
//         return KDevelop::VcsJob::JobFailed;
//     }
// }

// SvnUiDelegate* SvnKJobBase::ui()
// {
//     return static_cast<SvnUiDelegate*>(uiDelegate());
// }

// bool SvnKJobBase::requestKill()
// {
//     disconnect( d->m_th, 0, 0, 0 );
//     setErrorText( i18n("Job was terminated") );
//     return kill( KJob::EmitResult );
// }

void SvnKJobBase::threadFinished()
{
    // for VcsJob ifaces
    emit resultsReady( this );

    // for SubversionPart internals
    emitResult();
}

bool SvnKJobBase::doKill()
{
    if( !d->m_th ) return false;

// 	// disconnect so that threadFinished() slot doesn't get called later,
// 	// which will call emitResult() again.
// 	disconnect( d->m_th, 0, 0, 0 );
//     setErrorText( i18n("Job was terminated") );
    bool ret = d->m_th->requestTerminate();
    kDebug() << " SvnKJobBase::doKill() return value : " << ret << endl;
    return ret;
}

#include "svnkjobbase.moc"
