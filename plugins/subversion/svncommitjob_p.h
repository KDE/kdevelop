/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright 2007 Andreas Pakulat <apaku@gmx.de>                         *
 *   Copyright 2007 Dukju Ahn <dukjuahn@gmail.com>                         *
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

#ifndef KDEVPLATFORM_PLUGIN_SVNCOMMITJOB_P_H
#define KDEVPLATFORM_PLUGIN_SVNCOMMITJOB_P_H

#include <QString>
#include <kurl.h>
#include "svninternaljobbase.h"

class SvnJobBase;

class SvnInternalCommitJob : public SvnInternalJobBase
{
    Q_OBJECT
public:
    SvnInternalCommitJob( SvnJobBase* parent = 0 );
    void setRecursive( bool );
    void setCommitMessage( const QString& );
    void setUrls( const KUrl::List& );
    void setKeepLock( bool );

    KUrl::List urls() const;
    QString commitMessage() const;
    bool recursive() const;
    bool keepLock() const;

protected:
    void run();
private:
    KUrl::List m_urls;
    bool m_recursive;
    bool m_keepLock;
};


#endif

