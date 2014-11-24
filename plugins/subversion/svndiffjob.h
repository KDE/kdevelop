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

#ifndef KDEVPLATFORM_PLUGIN_SVNDIFFJOB_H
#define KDEVPLATFORM_PLUGIN_SVNDIFFJOB_H


#include "svnjobbase.h"

#include <vcs/vcslocation.h>

#include <QUrl>

#include <vcs/vcsdiff.h>

class SvnCatJob;

namespace KDevelop
{
    class VcsRevision;
}

class SvnInternalDiffJob;

class SvnDiffJob : public SvnJobBaseImpl<SvnInternalDiffJob>
{
    Q_OBJECT
public:
    SvnDiffJob( KDevSvnPlugin* parent );
    QVariant fetchResults();
    void start();
    void setSource( const KDevelop::VcsLocation& );
    void setDestination( const KDevelop::VcsLocation& );
    void setPegRevision( const KDevelop::VcsRevision& );
    void setSrcRevision( const KDevelop::VcsRevision& );
    void setDstRevision( const KDevelop::VcsRevision& );
    void setRecursive( bool );
    void setIgnoreAncestry( bool );
    void setIgnoreContentType( bool );
    void setNoDiffOnDelete( bool );

    void setDiffType( KDevelop::VcsDiff::Type type );

public slots:
    void setDiff( const QString& );
    void addLeftText( KDevelop::VcsJob* job );
    void removeJob( KJob* job );
private:
    KDevelop::VcsDiff m_diff;
    KDevelop::VcsDiff::Type m_diffType;
    QMap<KDevelop::VcsJob*, KDevelop::VcsLocation> m_catJobMap;
};


#endif

