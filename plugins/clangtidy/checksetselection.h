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

#ifndef CLANGTIDY_CHECKSETSELECTION_H
#define CLANGTIDY_CHECKSETSELECTION_H

// Qt
#include <QSharedDataPointer>

class QString;

namespace ClangTidy
{

class CheckSetSelectionPrivate;

class CheckSetSelection
{
public:
    CheckSetSelection();
    CheckSetSelection(const CheckSetSelection& other);
    ~CheckSetSelection();

public:
    CheckSetSelection& operator=(const CheckSetSelection& other);

public:
    void setId(const QString& id);
    QString id() const;

public:
    void setName(const QString& name);
    QString name() const;

public:
    QString selectionAsString() const;

    void setSelection(const QString& selection);

private:
    QSharedDataPointer<CheckSetSelectionPrivate> d;
};

}

#endif
