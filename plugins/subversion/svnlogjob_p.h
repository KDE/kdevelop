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

#ifndef KDEVPLATFORM_PLUGIN_SVNLOGJOB_P_H
#define KDEVPLATFORM_PLUGIN_SVNLOGJOB_P_H

#include "svninternaljobbase.h"

#include <vcs/vcsrevision.h>
#include <vcs/vcsevent.h>

class SvnInternalLogJob : public SvnInternalJobBase
{
    Q_OBJECT
public:
    SvnInternalLogJob( SvnJobBase* parent = 0 );

    void setLocation( const QUrl &location );
    void setLimit( int limit );
    void setEndRevision( const KDevelop::VcsRevision& rev );
    void setStartRevision( const KDevelop::VcsRevision& rev );

    QUrl location() const;
    KDevelop::VcsRevision startRevision() const;
    KDevelop::VcsRevision endRevision() const;
    int limit() const;
signals:
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
