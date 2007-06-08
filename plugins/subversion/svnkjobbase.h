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

#ifndef SVNKJOB_H
#define SVNKJOB_H

#include <kjob.h>
#include "vcshelpers.h"

class SubversionThread;
class QVariant;

class SvnKJobBase : public KJob, public KDevelop::VcsJob
{
    Q_OBJECT
public:
    friend class SubversionThread;

    SvnKJobBase( int type, QObject *parent );
    virtual ~SvnKJobBase();

    void setResult( const QVariant &result );
    virtual QVariant fetchResults();

    void setSvnThread( SubversionThread *job );
    SubversionThread *svnThread();
    KDevelop::VcsJob::Type type();

    QString smartError(); // subversion internal
    QString errorMessage(); // VcsJob iface

    virtual void start();
    FinishStatus exec();
//     SvnUiDelegate* ui();

Q_SIGNALS:
    void resultsReady( VcsJob* );
    void finished( VcsJob*, FinishStatus );

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
