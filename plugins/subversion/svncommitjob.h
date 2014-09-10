/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright 2007 Andreas Pakulat <apaku@gmx.de>                     *
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

#ifndef KDEVPLATFORM_PLUGIN_SVNCOMMITJOB_H
#define KDEVPLATFORM_PLUGIN_SVNCOMMITJOB_H

#include "svnjobbase.h"
#include <QUrl>
#include <QString>
#include <ThreadWeaver/Job>

class SvnInternalCommitJob;
class SvnCommitDialog;

class SvnCommitJob : public SvnJobBase
{
    Q_OBJECT
public:
    explicit SvnCommitJob( KDevSvnPlugin* parent );
    QVariant fetchResults();
    void start();
    void setCommitMessage( const QString& msg );
    void setKeepLock( bool );
    void setUrls( const QList<QUrl>& urls );
    void setRecursive( bool );
    SvnInternalJobBase* internalJob() const;
private:
    SvnInternalCommitJob* m_job;
};

#endif

