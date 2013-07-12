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
#ifndef KDEVPLATFORM_LANGUAGE_H
#define KDEVPLATFORM_LANGUAGE_H

#include <interfaces/ilanguage.h>

#include <QtCore/QList>

#include "shellexport.h"

namespace KDevelop {

class KDEVPLATFORMSHELL_EXPORT Language: public ILanguage {
    Q_OBJECT
public:
    explicit Language(ILanguageSupport *support, QObject *parent = 0);
    virtual ~Language();

    virtual void activate();
    virtual void deactivate();

    virtual ILanguageSupport *languageSupport();

    /**
     * Every thread that does background-parsing should read-lock its language's parse-mutex while parsing.
     * Any other thread may write-lock the parse-mutex in order to wait for all parsing-threads to finish the parsing.
     * The parse-mutex only needs to be locked while working on the du-chain, not while preprocessing or reading.
     */
    virtual QReadWriteLock* parseLock() const;

private:
    struct LanguagePrivate *d;
};

}

#endif

