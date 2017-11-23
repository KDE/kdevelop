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

#include "vcsdiff.h"

#include <QString>
#include <QUrl>
#include <QSharedData>

namespace KDevelop
{

class VcsDiffPrivate : public QSharedData
{
public:
    QUrl baseDiff;
    QString diff;
    uint depth = 0;
};

VcsDiff::VcsDiff()
    : d(new VcsDiffPrivate)
{
}

VcsDiff::~VcsDiff() = default;

VcsDiff::VcsDiff( const VcsDiff& rhs )
    : d(rhs.d)
{
}

bool VcsDiff::isEmpty() const
{
    return d->diff.isEmpty();
}

QString VcsDiff::diff() const
{
    return d->diff;
}


void VcsDiff::setDiff( const QString& s )
{
    d->diff = s;
}

VcsDiff& VcsDiff::operator=( const VcsDiff& rhs)
{
    d = rhs.d;
    return *this;
}

QUrl VcsDiff::baseDiff() const
{
    return d->baseDiff;
}

uint VcsDiff::depth() const
{
    return d->depth;
}

void VcsDiff::setBaseDiff(const QUrl& url)
{
    d->baseDiff=url;
}

void VcsDiff::setDepth(const uint depth)
{
    d->depth = depth;
}


}

