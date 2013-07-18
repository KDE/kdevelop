/***************************************************************************
 *   Copyright 2007 Alexander Dymo  <adymo@kdevelop.org>            *
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
#ifndef KDEVPLATFORM_ILANGUAGE_H
#define KDEVPLATFORM_ILANGUAGE_H

#include <QtCore/QObject>

#include <kurl.h>

#include "interfacesexport.h"

class QReadWriteLock;
class QThread;

namespace KDevelop {

class ILanguageSupport;
class BackgroundParser;

class KDEVPLATFORMINTERFACES_EXPORT ILanguage: public QObject {
public:
    explicit ILanguage(const QString &name, QObject *parent = 0);
    virtual ~ILanguage();

    QString name() const;

    virtual void activate() = 0;
    virtual void deactivate() = 0;

    /** @return the language support plugin.*/
    virtual ILanguageSupport *languageSupport() = 0;

    /**
     * Every thread that does background-parsing should read-lock its language's parse-mutex while parsing.
     * Any other thread may write-lock the parse-mutex in order to wait for all parsing-threads to finish the parsing.
     * The parse-mutex only needs to be locked while working on the du-chain, not while preprocessing or reading.
     * Tip: use QReadLocker for read-locking.
     * The duchain must always be unlocked when you try to lock a parseLock!
     */
    virtual QReadWriteLock* parseLock() const = 0;

private:
    struct ILanguagePrivate *d;
};

}

#endif

