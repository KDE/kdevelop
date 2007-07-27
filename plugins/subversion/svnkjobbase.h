/***************************************************************************
 *   Copyright 2007 Dukju Ahn <dukjuahn@gmail.com>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef SVNKJOB_H
#define SVNKJOB_H

#include <kjob.h>
#include "vcshelpers.h"
#include "vcsjob.h"

class SubversionThread;
class SubversionCore;
class QVariant;

class SvnKJobBase : public KDevelop::VcsJob
{
    Q_OBJECT
public:
    friend class SubversionThread;

    SvnKJobBase( int type, SubversionCore *parent );
    virtual ~SvnKJobBase();

    JobStatus status();

    void setResult( const QVariant &result );
    virtual QVariant fetchResults();

    void setSvnThread( SubversionThread *job );
    SubversionThread *svnThread();
    KDevelop::VcsJob::JobType type();
    SubversionCore *svncore();

    QString smartError(); // subversion internal

    virtual void start();
//     JobStatus exec();
//     SvnUiDelegate* ui();

Q_SIGNALS:
    void resultsReady( VcsJob* );

protected Q_SLOTS:
    void threadFinished();

protected:
    // The forceful termination of thread causes deadlock in some cases.
    // Currently not used.
    // TODO
    virtual bool doKill();

protected:
    class Private;
    Private *const d;
};

#endif
