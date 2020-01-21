/*
 * This file is part of KDevelop
 *
 * Copyright 2020 Friedrich W. H. Kossebau <kossebau@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include "checksetselection.h"

// Qt
#include <QString>

namespace Clazy
{

class CheckSetSelectionPrivate : public QSharedData
{
public:
    QString id;
    QString name;

    QString selection;
};


CheckSetSelection::CheckSetSelection()
    : d(new CheckSetSelectionPrivate)
{
}
CheckSetSelection::CheckSetSelection(const CheckSetSelection& other) = default;

CheckSetSelection::~CheckSetSelection() = default;

CheckSetSelection& CheckSetSelection::operator=(const CheckSetSelection& other) = default;

QString CheckSetSelection::selectionAsString() const
{
    return d->selection;
}

QString CheckSetSelection::id() const
{
    return d->id;
}

QString CheckSetSelection::name() const
{
    return d->name;
}

void CheckSetSelection::setId(const QString& id)
{
    d->id = id;
}

void CheckSetSelection::setSelection(const QString& selection)
{
    d->selection = selection;
}

void CheckSetSelection::setName(const QString& name)
{
    d->name = name;
}

}
