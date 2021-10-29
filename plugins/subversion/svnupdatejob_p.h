/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_SVNUPDATEJOB_P_H
#define KDEVPLATFORM_PLUGIN_SVNUPDATEJOB_P_H

#include "svninternaljobbase.h"

class SvnInternalUpdateJob : public SvnInternalJobBase
{
    Q_OBJECT
public:
    explicit SvnInternalUpdateJob( SvnJobBase* parent = nullptr );
    void setLocations( const QList<QUrl>& );
    void setRecursive( bool );
    void setRevision( const KDevelop::VcsRevision& );
    void setIgnoreExternals( bool );

    bool recursive() const;
    QList<QUrl> locations() const;
    KDevelop::VcsRevision revision() const;
    bool ignoreExternals() const;
protected:
    void run(ThreadWeaver::JobPointer self, ThreadWeaver::Thread* thread) override;
private:
    QList<QUrl> m_locations;
    bool m_recursive = false;
    bool m_ignoreExternals = false;
    KDevelop::VcsRevision m_revision;
};

#endif

