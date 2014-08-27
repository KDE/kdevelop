/***************************************************************************
 *   Copyright 2006 Hamish Rodda <rodda@kde.org>                       *
 *   Copyright 2007 Alexander Dymo <adymo@kdevelop.org>             *
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
#include "language.h"

#include <QHash>
#include <QReadWriteLock>
#include <QThread>

#include <kdebug.h>

#include <language/interfaces/ilanguagesupport.h>

namespace KDevelop {

struct LanguagePrivate {
    ILanguageSupport *support;

    mutable QReadWriteLock lock;
};

Language::Language(ILanguageSupport *support, QObject *parent)
    : ILanguage(support->name(), parent)
{
    kDebug() << "creating language" << support->name();

    d = new LanguagePrivate;
    d->support = support;
}

Language::~Language()
{
    delete d;
}

void Language::deactivate()
{
    kDebug() << "deactivating language" << name();
}

void Language::activate()
{
    kDebug() << "activating language" << name();
}

ILanguageSupport *Language::languageSupport()
{
    return d->support;
}

QReadWriteLock *Language::parseLock() const
{
    return &d->lock;
}

}


