/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "vcsstatusinfo.h"

#include <QDebug>
#include <QUrl>
#include <QSharedData>

namespace KDevelop
{

class VcsStatusInfoPrivate : public QSharedData
{
public:
    int state;
    int extendedState;
    QUrl url;
};

VcsStatusInfo::VcsStatusInfo()
    : d( new VcsStatusInfoPrivate)
{
    d->state = VcsStatusInfo::ItemUnknown;
    d->extendedState = VcsStatusInfo::ItemUnknown;
}

VcsStatusInfo::~VcsStatusInfo() = default;

VcsStatusInfo::VcsStatusInfo( const VcsStatusInfo& rhs )
    : d(rhs.d)
{
}

VcsStatusInfo& VcsStatusInfo::operator=( const VcsStatusInfo& rhs)
{
    d = rhs.d;
    return *this;
}

bool VcsStatusInfo::operator==( const KDevelop::VcsStatusInfo& rhs) const
{
    return ( d->state == rhs.d->state && d->url == rhs.d->url );
}

bool VcsStatusInfo::operator!=( const KDevelop::VcsStatusInfo& rhs) const
{
    return !(operator==(rhs));
}

void VcsStatusInfo::setUrl( const QUrl& url )
{
    d->url = url;
}

void VcsStatusInfo::setExtendedState( int newstate )
{
    d->extendedState = newstate;
}

void VcsStatusInfo::setState( VcsStatusInfo::State state )
{
    d->state = state;
}

int VcsStatusInfo::extendedState() const
{
    return d->extendedState;
}

QUrl VcsStatusInfo::url() const
{
    return d->url;
}

VcsStatusInfo::State VcsStatusInfo::state() const
{
    return VcsStatusInfo::State(d->state);
}

}

QDebug operator<<(QDebug s, const KDevelop::VcsStatusInfo& statusInfo)
{
    s.nospace() << statusInfo.state() << "@" << statusInfo.url();
    return s.space();
}
