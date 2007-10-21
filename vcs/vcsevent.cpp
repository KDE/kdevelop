/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright 2007 Andreas Pakulat <apaku@gmx.de>                     *
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

#include "vcsevent.h"
#include "vcsrevision.h"
#include <QtCore/QString>
#include <QtCore/QList>
#include <QtCore/QDateTime>

namespace KDevelop
{

class VcsItemEventPrivate
{
public:
    QString location;
    QString sourceLocation;
    VcsRevision revision;
    VcsRevision sourceRevision;
    VcsItemEvent::Actions actions;
};

VcsItemEvent::VcsItemEvent()
    : d(new VcsItemEventPrivate)
{
}

VcsItemEvent::~VcsItemEvent()
{
    delete d;
}

VcsItemEvent::VcsItemEvent(const VcsItemEvent& rhs )
    : d(new VcsItemEventPrivate)
{
    d->actions = rhs.d->actions;
    d->revision = rhs.d->revision;
    d->sourceRevision = rhs.d->sourceRevision;
    d->sourceLocation = rhs.d->sourceLocation;
    d->location = rhs.d->location;
}

QString VcsItemEvent::repositoryLocation() const
{
    return d->location;
}

QString VcsItemEvent::repositoryCopySourceLocation() const
{
    return d->sourceLocation;
}

VcsRevision VcsItemEvent::repositoryCopySourceRevision() const
{
    return d->sourceRevision;
}

VcsRevision VcsItemEvent::revision() const
{
    return d->revision;
}

VcsItemEvent::Actions VcsItemEvent::actions() const
{
    return d->actions;
}

void VcsItemEvent::setRepositoryLocation( const QString& l )
{
    d->location = l;
}

void VcsItemEvent::setRepositorCopySourceLocation( const QString& l )
{
    d->sourceLocation = l;
}

void VcsItemEvent::setRevision( const KDevelop::VcsRevision& rev )
{
    d->revision = rev;
}

void VcsItemEvent::setRepositorCopySourceRevision( const KDevelop::VcsRevision& rev )
{
    d->sourceRevision = rev;
}

void VcsItemEvent::setActions( VcsItemEvent::Actions a )
{
    d->actions = a;
}

VcsItemEvent& VcsItemEvent::operator=( const VcsItemEvent& rhs)
{
    if(this == &rhs)
        return *this;
    d->actions = rhs.d->actions;
    d->revision = rhs.d->revision;
    d->sourceRevision = rhs.d->sourceRevision;
    d->sourceLocation = rhs.d->sourceLocation;
    d->location = rhs.d->location;
    return *this;
}

class VcsEventPrivate
{
public:
    VcsRevision revision;
    QString author;
    QString message;
    QDateTime date;
    VcsItemEvent::Actions actions;
    QList<VcsItemEvent> items;
};

VcsEvent::VcsEvent()
    : d(new VcsEventPrivate)
{
}

VcsEvent::~VcsEvent()
{
    delete d;
}

VcsEvent::VcsEvent( const VcsEvent& rhs )
    : d(new VcsEventPrivate)
{
    d->revision = rhs.d->revision;
    d->author = rhs.d->author;
    d->message = rhs.d->message;
    d->date = rhs.d->date;
    d->actions = rhs.d->actions;
    d->items = rhs.d->items;
}

VcsRevision VcsEvent::revision()
{
    return d->revision;
}

QString VcsEvent::author()
{
    return d->author;
}

QDateTime VcsEvent::date()
{
    return d->date;
}

QString VcsEvent::message()
{
    return d->message;
}

VcsItemEvent::Actions VcsEvent::actions()
{
    return d->actions;
}

QList<VcsItemEvent> VcsEvent::items()
{
    return d->items;
}

void VcsEvent::setRevision( const VcsRevision& rev )
{
    d->revision = rev;
}

void VcsEvent::setAuthor( const QString& a )
{
    d->author = a;
}

void VcsEvent::setDate( const QDateTime& date )
{
    d->date = date;
}

void VcsEvent::setMessage(const QString& m )
{
    d->message = m;
}

void VcsEvent::setActions( VcsItemEvent::Actions a )
{
    d->actions = a;
}

void VcsEvent::setItems( const QList<VcsItemEvent>& l )
{
    d->items = l;
}

VcsEvent& VcsEvent::operator=( const VcsEvent& rhs)
{
    if(this == &rhs)
        return *this;
    d->actions = rhs.d->actions;
    d->revision = rhs.d->revision;
    d->message = rhs.d->message;
    d->items = rhs.d->items;
    d->date = rhs.d->date;
    d->author = rhs.d->author;
    return *this;
}


}

