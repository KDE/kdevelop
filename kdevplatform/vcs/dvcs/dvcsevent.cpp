/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>
    SPDX-FileCopyrightText: 2007 Matthew Woehlke <mw_triad@users.sourceforge.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "dvcsevent.h"

#include <QSharedData>
#include <QStringList>

using namespace KDevelop;

class KDevelop::DVcsEventPrivate : public QSharedData
{
public:
    int type;

    QString commit;
    QStringList parents;
    QString date;
    QString author;
    QString log;
    QList<int> properties; //used to describe graph columns in every row (MERGE, HEAD, CROSS, etc)
};

DVcsEvent::DVcsEvent()
    : d(new DVcsEventPrivate)
{
}

DVcsEvent::~DVcsEvent() = default;

DVcsEvent::DVcsEvent(const DVcsEvent& rhs)
    : d(rhs.d)
{
}

DVcsEvent& DVcsEvent::operator=(const DVcsEvent& rhs)
{
    d = rhs.d;
    return *this;
}

QString DVcsEvent::commit() const
{
    return d->commit;
}

void DVcsEvent::setCommit(const QString& commit)
{
    d->commit = commit;
}

QStringList DVcsEvent::parents() const
{
    return d->parents;
}

void DVcsEvent::setParents(const QStringList& parents)
{
    d->parents = parents;
    switch (d->parents.count()) {
    case 0:
    {
        setType(INITIAL);
        break;
    }
    case 1:
    {
        setType(BRANCH);
        break;
    }
    default: // > 1
    {
        setType(MERGE);
        break;
    }
    }
}

QString DVcsEvent::date() const
{
    return d->date;
}

void DVcsEvent::setDate(const QString& date)
{
    d->date = date;
}

QString DVcsEvent::author() const
{
    return d->author;
}

void DVcsEvent::setAuthor(const QString& author)
{
    d->author = author;
}

QString DVcsEvent::log() const
{
    return d->log;
}

void DVcsEvent::setLog(const QString& log)
{
    d->log = log;
}

int DVcsEvent::type() const
{
    return d->type;
}

void DVcsEvent::setType(CommitType type)
{
    d->type = type;
}

QList<int> DVcsEvent::properties() const
{
    return d->properties;
}

void DVcsEvent::setProperties(const QList<int>& properties)
{
    d->properties = properties;
}

void DVcsEvent::setProperty(int index, int prop)
{
    if (index >= 0 && index < d->properties.count())
        d->properties[index] = prop;
}
