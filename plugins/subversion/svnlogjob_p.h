/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_SVNLOGJOB_P_H
#define KDEVPLATFORM_PLUGIN_SVNLOGJOB_P_H

#include "svninternaljobbase.h"

#include <vcs/vcsrevision.h>
#include <vcs/vcsevent.h>

class SvnInternalLogJob : public SvnInternalJobBase
{
    Q_OBJECT
public:
    explicit SvnInternalLogJob( SvnJobBase* parent = nullptr );

    void setLocation( const QUrl &location );
    void setLimit( int limit );
    void setEndRevision( const KDevelop::VcsRevision& rev );
    void setStartRevision( const KDevelop::VcsRevision& rev );

    QUrl location() const;
    KDevelop::VcsRevision startRevision() const;
    KDevelop::VcsRevision endRevision() const;
    int limit() const;
Q_SIGNALS:
    void logEvent( const KDevelop::VcsEvent& );
protected:
    void run(ThreadWeaver::JobPointer self, ThreadWeaver::Thread* thread) override;
private:
    QUrl m_location;
    int m_limit;
    KDevelop::VcsRevision m_startRevision;
    KDevelop::VcsRevision m_endRevision;
};



#endif
