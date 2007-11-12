/* This file is part of KDevelop
Copyright 2007 Hamish Rodda <rodda@kde.org>

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
the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
Boston, MA 02110-1301, USA.
*/

#include "iproblem.h"

using namespace KDevelop;

class Problem::Private
{
public:
    DocumentRange finalLocation;
    QStack<DocumentCursor> locationStack;
    QString description;
    QString explanation;
};

Problem::Problem()
    : d(new Private)
{
}

Problem::Problem(const Problem& other)
    : d(new Private(*other.d))
{
}

Problem::~Problem()
{
    delete d;
}

const DocumentRange & Problem::finalLocation() const
{
    return d->finalLocation;
}

void Problem::setFinalLocation(const DocumentRange & location)
{
    d->finalLocation.setRange(location);
    d->finalLocation.setDocument(location.document());
}

const QStack< DocumentCursor > & Problem::locationStack() const
{
    return d->locationStack;
}

void Problem::addLocation(const DocumentCursor & cursor)
{
    d->locationStack.push(DocumentCursor(cursor));
}

void Problem::clearLocationStack()
{
    d->locationStack.clear();
}

const QString & Problem::description() const
{
    return d->description;
}

void Problem::setDescription(const QString & description)
{
    d->description = description;
}

const QString & Problem::explanation() const
{
    return d->explanation;
}

void Problem::setExplanation(const QString & explanation)
{
    d->explanation = explanation;
}

Problem& Problem::operator=(const Problem& rhs)
{
    *d = *rhs.d;
    return *this;
}

/*
  public QSharedData
  QSharedDataPointer<Data> d;
*/
