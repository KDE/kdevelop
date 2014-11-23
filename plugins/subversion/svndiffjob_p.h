/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright 2007 Andreas Pakulat <apaku@gmx.de>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#ifndef KDEVPLATFORM_PLUGIN_SVNDIFFJOB_P_H
#define KDEVPLATFORM_PLUGIN_SVNDIFFJOB_P_H

#include "svninternaljobbase.h"
#include <QVariant>
#include <vcs/vcsrevision.h>

class SvnInternalDiffJob : public SvnInternalJobBase
{
    Q_OBJECT
public:
    SvnInternalDiffJob( SvnJobBase* parent = 0 );
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

signals:
    void gotDiff( const QString& );
protected:
    void run(ThreadWeaver::JobPointer self, ThreadWeaver::Thread* thread) override;
private:
    KDevelop::VcsLocation m_source;
    KDevelop::VcsLocation m_destination;
    KDevelop::VcsRevision m_srcRevision;
    KDevelop::VcsRevision m_dstRevision;
    KDevelop::VcsRevision m_pegRevision;
    bool m_recursive;
    bool m_ignoreAncestry;
    bool m_ignoreContentType;
    bool m_noDiffOnDelete;
};


#endif

