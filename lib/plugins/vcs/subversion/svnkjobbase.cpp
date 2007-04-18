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
#include <klocale.h>

class SvnKJobBase::Private
{
public:
    SubversionThread *m_th;
    int m_type;
};

SvnKJobBase::SvnKJobBase( int type, QObject *parent )
    : KJob( parent ), d( new Private )
{
    d->m_th = 0;
    d->m_type = type;
    
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

void SvnKJobBase::setSvnThread( SubversionThread *th )
{
    d->m_th = th;
    connect( d->m_th, SIGNAL(finished()), this, SLOT(threadFinished()) );
}

SubversionThread* SvnKJobBase::svnThread()
{
    return d->m_th;
}

int SvnKJobBase::type()
{
    return d->m_type;
}

QString SvnKJobBase::smartError()
{
    QString msg = errorText();
    if( msg.isEmpty() ){
        msg = i18n("An error occured during subversion operation");
    }
    return msg;
}

void SvnKJobBase::start()
{
    if( !d->m_th ) return;
    d->m_th->start();
}

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
