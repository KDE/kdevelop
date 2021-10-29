/*
    SPDX-FileCopyrightText: 2020 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "checksetselection.h"

// Qt
#include <QString>

namespace ClangTidy
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
