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

#ifndef SVNINFOJOB_P_H
#define SVNINFOJOB_P_H

#include "svninternaljobbase.h"
#include "svninfojob.h"

class SvnInfoJobHelper : public QObject
{
    Q_OBJECT
public:
    void emitInfo( const SvnInfoHolder & state )
    {
        emit gotInfo( state );
    }
signals:
    void gotInfo( const SvnInfoHolder& );

};

class SvnInternalInfoJob : public SvnInternalJobBase
{
    Q_OBJECT
public:
    SvnInternalInfoJob( SvnJobBase* parent = 0 );
    void setLocation( const KUrl& );

    KUrl location() const;
signals:
    void gotInfo( const SvnInfoHolder& );
protected:
    void run();
private:
    KUrl m_location;
};


#endif

