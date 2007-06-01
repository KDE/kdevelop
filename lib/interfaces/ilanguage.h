/***************************************************************************
 *   Copyright (C) 2007 by Alexander Dymo  <adymo@kdevelop.org>            *
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
#ifndef ILANGUAGE_H
#define ILANGUAGE_H

#include <QtCore/QObject>

#include <kurl.h>

#include "interfacesexport.h"

class QMutex;
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

    //virtual CodeDelegate *codeDelegate() const = 0;
    //virtual CodeHighlighting *codeHighlighting() const = 0;

    /** @return the language support plugin.*/
    virtual ILanguageSupport *languageSupport() = 0;

    /** @return the background parser for parsing sources.*/
    virtual BackgroundParser *backgroundParser() = 0;

    virtual QMutex *parseMutex(QThread *thread) const = 0;
    virtual void lockAllParseMutexes() = 0;
    virtual void unlockAllParseMutexes() = 0;

private:
    struct ILanguagePrivate *d;
};

}

#endif

//kate: space-indent on; indent-width 4; tab-width: 4; replace-tabs on;
