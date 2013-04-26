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

#ifndef KDEVPLATFORM_PLUGIN_SVNUPDATEJOB_P_H
#define KDEVPLATFORM_PLUGIN_SVNUPDATEJOB_P_H

#include "svninternaljobbase.h"

class SvnInternalUpdateJob : public SvnInternalJobBase
{
    Q_OBJECT
public:
    SvnInternalUpdateJob( SvnJobBase* parent = 0 );
    void setLocations( const KUrl::List& );
    void setRecursive( bool );
    void setRevision( const KDevelop::VcsRevision& );
    void setIgnoreExternals( bool );

    bool recursive() const;
    KUrl::List locations() const;
    KDevelop::VcsRevision revision() const;
    bool ignoreExternals() const;
protected:
    void run();
private:
    KUrl::List m_locations;
    bool m_recursive;
    bool m_ignoreExternals;
    KDevelop::VcsRevision m_revision;
};

#endif

