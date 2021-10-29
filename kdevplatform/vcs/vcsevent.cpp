/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "vcsevent.h"
#include <QString>
#include <QList>
#include <QDateTime>
#include <QVariant>
#include <QSharedData>

#include "vcsrevision.h"
namespace KDevelop
{

class VcsItemEventPrivate : public QSharedData
{
public:
    QString location;
    QString sourceLocation;
    VcsRevision sourceRevision;
    VcsItemEvent::Actions actions;
};

VcsItemEvent::VcsItemEvent()
    : d(new VcsItemEventPrivate)
{
}

VcsItemEvent::~VcsItemEvent() = default;

VcsItemEvent::VcsItemEvent(const VcsItemEvent& rhs )
    : d(rhs.d)
{
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

VcsItemEvent::Actions VcsItemEvent::actions() const
{
    return d->actions;
}

void VcsItemEvent::setRepositoryLocation( const QString& l )
{
    d->location = l;
}

void VcsItemEvent::setRepositoryCopySourceLocation( const QString& l )
{
    d->sourceLocation = l;
}

void VcsItemEvent::setRepositoryCopySourceRevision( const KDevelop::VcsRevision& rev )
{
    d->sourceRevision = rev;
}

void VcsItemEvent::setActions( VcsItemEvent::Actions a )
{
    d->actions = a;
}

VcsItemEvent& VcsItemEvent::operator=( const VcsItemEvent& rhs)
{
    d = rhs.d;
    return *this;
}

class VcsEventPrivate : public QSharedData
{
public:
    VcsRevision revision;
    QString author;
    QString message;
    QDateTime date;
    QList<VcsItemEvent> items;
};

VcsEvent::VcsEvent()
    : d(new VcsEventPrivate)
{
}

VcsEvent::~VcsEvent() = default;

VcsEvent::VcsEvent( const VcsEvent& rhs )
    : d(rhs.d)
{
}

VcsRevision VcsEvent::revision() const
{
    return d->revision;
}

QString VcsEvent::author() const
{
    return d->author;
}

QDateTime VcsEvent::date() const
{
    return d->date;
}

QString VcsEvent::message() const
{
    return d->message;
}

QList<VcsItemEvent> VcsEvent::items() const
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

void VcsEvent::setItems( const QList<VcsItemEvent>& l )
{
    d->items = l;
}

void VcsEvent::addItem(const VcsItemEvent& item)
{
    d->items.append(item);
}

VcsEvent& VcsEvent::operator=( const VcsEvent& rhs)
{
    d = rhs.d;
    return *this;
}


}

