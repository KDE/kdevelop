/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_SVNDIFFJOB_P_H
#define KDEVPLATFORM_PLUGIN_SVNDIFFJOB_P_H

#include "svninternaljobbase.h"
#include <vcs/vcsrevision.h>

class SvnInternalDiffJob : public SvnInternalJobBase
{
    Q_OBJECT
public:
    explicit SvnInternalDiffJob( SvnJobBase* parent = nullptr );
    void setSource( const KDevelop::VcsLocation& );
    void setDestination( const KDevelop::VcsLocation& );
    void setSrcRevision( const KDevelop::VcsRevision& );
    void setDstRevision( const KDevelop::VcsRevision& );
    void setPegRevision( const KDevelop::VcsRevision& );
    void setRecursive( bool );
    void setIgnoreAncestry( bool );
    void setIgnoreContentType( bool );
    void setNoDiffOnDelete( bool );

    bool recursive() const;
    bool ignoreAncestry() const;
    bool ignoreContentType() const;
    bool noDiffOnDelete() const;
    KDevelop::VcsLocation source() const;
    KDevelop::VcsLocation destination() const;
    KDevelop::VcsRevision srcRevision() const;
    KDevelop::VcsRevision dstRevision() const;
    KDevelop::VcsRevision pegRevision() const;

Q_SIGNALS:
    void gotDiff( const QString& );
protected:
    void run(ThreadWeaver::JobPointer self, ThreadWeaver::Thread* thread) override;
private:
    KDevelop::VcsLocation m_source;
    KDevelop::VcsLocation m_destination;
    KDevelop::VcsRevision m_srcRevision;
    KDevelop::VcsRevision m_dstRevision;
    KDevelop::VcsRevision m_pegRevision;
    bool m_recursive = true;
    bool m_ignoreAncestry = false;
    bool m_ignoreContentType = false;
    bool m_noDiffOnDelete = false;
};


#endif

