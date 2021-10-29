/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_SVNBLAMEJOB_P_H
#define KDEVPLATFORM_PLUGIN_SVNBLAMEJOB_P_H


#include "svninternaljobbase.h"

#include <vcs/vcsrevision.h>

namespace KDevelop {
    class VcsAnnotationLine;
}

class QUrl;
class SvnInternalBlameJob : public SvnInternalJobBase
{
    Q_OBJECT
public:
    explicit SvnInternalBlameJob( SvnJobBase* parent = nullptr );

    void setLocation( const QUrl &location );
    void setEndRevision( const KDevelop::VcsRevision& rev );
    void setStartRevision( const KDevelop::VcsRevision& rev );

    QUrl location() const;
    KDevelop::VcsRevision startRevision() const;
    KDevelop::VcsRevision endRevision() const;
Q_SIGNALS:
    void blameLine( const KDevelop::VcsAnnotationLine& );
protected:
    void run(ThreadWeaver::JobPointer self, ThreadWeaver::Thread* thread) override;
private:
    QUrl m_location;
    KDevelop::VcsRevision m_startRevision;
    KDevelop::VcsRevision m_endRevision;
};


#endif
