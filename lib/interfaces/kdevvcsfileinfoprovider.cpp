/* This file is part of the KDE project
   Copyright (C) 2003 Mario Scalas <mario.scalas@libero.it>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <kdevversioncontrol.h>
#include "kdevvcsfileinfoprovider.h"

///////////////////////////////////////////////////////////////////////////////
// struct VCSFileInfo
///////////////////////////////////////////////////////////////////////////////

QString VCSFileInfo::toString() const
{
    return "(" + fileName + ", " + workRevision + ", " + repoRevision + ", " + state2String( state ) + ")";
}

///////////////////////////////////////////////////////////////////////////////

QString VCSFileInfo::state2String( FileState state )
{
    switch (state)
    {
        case Added: return "added";
        case Uptodate: return "up-to-date";
        case Modified: return "modified";
        case Conflict: return "conflict";
        case Sticky: return "sticky";
        case NeedsPatch: return "needs patch";
        case NeedsCheckout: return "needs check-out";
        case Directory: return "directory";
        case Unknown:
        default:
            return "unknown";
    }
}

///////////////////////////////////////////////////////////////////////////////
// struct KDevVCSFileInfoProvider::Private
///////////////////////////////////////////////////////////////////////////////

struct KDevVCSFileInfoProvider::Private
{
    Private( KDevVersionControl *owner ) : m_owner( owner ) {}

    KDevVersionControl *m_owner;
};

///////////////////////////////////////////////////////////////////////////////
// class KDevVCSFileInfoProvider
///////////////////////////////////////////////////////////////////////////////

KDevVCSFileInfoProvider::KDevVCSFileInfoProvider( KDevVersionControl *parent, const char *name )
    : QObject( parent, name ), d( 0 ) //d(new Private( parent ))
{
}

///////////////////////////////////////////////////////////////////////////////

KDevVCSFileInfoProvider::~KDevVCSFileInfoProvider()
{
//    delete d;
}

///////////////////////////////////////////////////////////////////////////////

KDevVersionControl *KDevVCSFileInfoProvider::owner() const
{
//    return d->m_owner;
    return static_cast<KDevVersionControl *>( parent() );
}

#include "kdevvcsfileinfoprovider.moc"

