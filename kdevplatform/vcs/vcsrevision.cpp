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

#include "vcsrevision.h"

#include <QString>
#include <QStringList>
#include <QMap>
#include <QDateTime>


namespace KDevelop
{

VcsRevision VcsRevision::createSpecialRevision( KDevelop::VcsRevision::RevisionSpecialType _type )
{
    VcsRevision rev;
    rev.setRevisionValue( QVariant::fromValue<KDevelop::VcsRevision::RevisionSpecialType>( _type ), VcsRevision::Special );
    return rev;
}

class VcsRevisionPrivate : public QSharedData
{
    public:
        QVariant value;
        VcsRevision::RevisionType type;
        QMap<QString,QVariant> internalValues;
};

VcsRevision::VcsRevision()
    : d(new VcsRevisionPrivate)
{
    d->type = VcsRevision::Invalid;
}

VcsRevision::VcsRevision( const VcsRevision& rhs )
    : d(rhs.d)
{
}

VcsRevision::~VcsRevision() = default;

VcsRevision& VcsRevision::operator=( const VcsRevision& rhs)
{
    d = rhs.d;
    return *this;
}

void VcsRevision::setRevisionValue( const QVariant& rev, VcsRevision::RevisionType type )
{
    d->value = rev;
    d->type = type;
}

VcsRevision::RevisionType VcsRevision::revisionType() const
{
    return d->type;
}

VcsRevision::RevisionSpecialType VcsRevision::specialType() const
{
    Q_ASSERT(d->type==Special);
    return d->value.value<RevisionSpecialType>();
}

QVariant VcsRevision::revisionValue() const
{
    return d->value;
}

QStringList VcsRevision::keys() const
{
    return d->internalValues.keys();
}

QVariant VcsRevision::value(const QString& key) const
{
    if( d->internalValues.contains(key) )
    {
        return d->internalValues[key];
    }
    return QVariant();
}

void VcsRevision::setValue( const QString& key, const QVariant& value )
{
    d->internalValues[key] = value;
}

void VcsRevision::setType( RevisionType t)
{
    d->type = t;
}

void VcsRevision::setSpecialType( RevisionSpecialType t)
{
    d->value = QVariant(t);
}

void VcsRevision::setValue( const QVariant& v )
{
    d->value = v;
}

bool VcsRevision::operator==( const KDevelop::VcsRevision& rhs ) const
{
    return ( d->type == rhs.d->type && d->value == rhs.d->value && d->internalValues == rhs.d->internalValues );
}

QString VcsRevision::prettyValue() const
{
    switch( revisionType() )
    {
        case GlobalNumber:
        case FileNumber:
            return (revisionValue().type() == QVariant::String ? revisionValue().toString() : QString::number(revisionValue().toLongLong()));
            break;
        case Special:
            switch( revisionValue().value<KDevelop::VcsRevision::RevisionSpecialType>(  ) )
            {
                case VcsRevision::Head:
                    return QStringLiteral("Head");
                    break;
                case VcsRevision::Base:
                    return QStringLiteral("Base");
                    break;
                case VcsRevision::Working:
                    return QStringLiteral("Working");
                    break;
                case VcsRevision::Previous:
                    return QStringLiteral("Previous");
                    break;
                case VcsRevision::Start:
                    return QStringLiteral("Start");
                    break;
                default:
                    return QStringLiteral("User");
                    break;
            }
            break;
        case Date:
            return revisionValue().toDateTime().toString( Qt::LocalDate );
            break;
        default:
            return revisionValue().toString();
            break;
    }
}

}

uint KDevelop::qHash( const KDevelop::VcsRevision& rev)
{
    const auto revisionValue = rev.revisionValue();
    switch (rev.revisionType()) {
        case VcsRevision::GlobalNumber:
        case VcsRevision::FileNumber:
            return (revisionValue.type() == QVariant::String ? ::qHash(revisionValue.toString()) :
                    ::qHash(revisionValue.toULongLong()));
            break;
        case VcsRevision::Special:
            return ::qHash(static_cast<int>(revisionValue.value<KDevelop::VcsRevision::RevisionSpecialType>()));
            break;
        case VcsRevision::Date:
            return ::qHash(revisionValue.toDateTime());
            break;
        default:
            break;
    }
    return ::qHash(revisionValue.toString());
}

