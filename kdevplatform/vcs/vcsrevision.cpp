/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "vcsrevision.h"

#include <QString>
#include <QStringList>
#include <QMap>
#include <QDateTime>
#include <QLocale>


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
    const auto valueIt = d->internalValues.constFind(key);
    if (valueIt != d->internalValues.constEnd()) {
        return *valueIt;
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
            return (revisionValue().typeId() == qMetaTypeId<QString>() ? revisionValue().toString()
                                                                       : QString::number(revisionValue().toLongLong()));
        case Special:
            switch( revisionValue().value<KDevelop::VcsRevision::RevisionSpecialType>(  ) )
            {
                case VcsRevision::Head:
                    return QStringLiteral("Head");
                case VcsRevision::Base:
                    return QStringLiteral("Base");
                case VcsRevision::Working:
                    return QStringLiteral("Working");
                case VcsRevision::Previous:
                    return QStringLiteral("Previous");
                case VcsRevision::Start:
                    return QStringLiteral("Start");
                default:
                    return QStringLiteral("User");
            }
        case Date:
            return  QLocale().toString(revisionValue().toDateTime(), QLocale::ShortFormat);
        default:
            return revisionValue().toString();
    }
}

}

size_t KDevelop::qHash(const KDevelop::VcsRevision& rev)
{
    const auto revisionValue = rev.revisionValue();
    switch (rev.revisionType()) {
        case VcsRevision::GlobalNumber:
        case VcsRevision::FileNumber:
            return (revisionValue.typeId() == qMetaTypeId<QString>() ? ::qHash(revisionValue.toString())
                                                                     : ::qHash(revisionValue.toULongLong()));
        case VcsRevision::Special:
            return ::qHash(static_cast<int>(revisionValue.value<KDevelop::VcsRevision::RevisionSpecialType>()));
        case VcsRevision::Date:
            return ::qHash(revisionValue.toDateTime());
        default:
            return ::qHash(revisionValue.toString());
    }
}

