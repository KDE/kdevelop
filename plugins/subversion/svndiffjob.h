/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_SVNDIFFJOB_H
#define KDEVPLATFORM_PLUGIN_SVNDIFFJOB_H


#include "svnjobbase.h"

#include <vcs/vcsdiff.h>

namespace KDevelop
{
    class VcsRevision;
    class VcsLocation;
}

class SvnInternalDiffJob;

class SvnDiffJob : public SvnJobBaseImpl<SvnInternalDiffJob>
{
    Q_OBJECT
public:
    explicit SvnDiffJob( KDevSvnPlugin* parent );
    QVariant fetchResults() override;
    void start() override;
    void setSource( const KDevelop::VcsLocation& );
    void setDestination( const KDevelop::VcsLocation& );
    void setPegRevision( const KDevelop::VcsRevision& );
    void setSrcRevision( const KDevelop::VcsRevision& );
    void setDstRevision( const KDevelop::VcsRevision& );
    void setRecursive( bool );
    void setIgnoreAncestry( bool );
    void setIgnoreContentType( bool );
    void setNoDiffOnDelete( bool );

public Q_SLOTS:
    void setDiff( const QString& );

private:
    KDevelop::VcsDiff m_diff;
};


#endif

