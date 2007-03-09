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
#ifndef ILANGUAGESUPPORT_H
#define ILANGUAGESUPPORT_H

#include <QtDesigner/QAbstractExtensionFactory>

#include "iextension.h"
#include "kdevexport.h"

namespace KDevelop {

class ParseJob;

class KDEVPLATFORM_EXPORT ILanguageSupport {
public:
    virtual ~ILanguageSupport() {}

    /** @return the name of the language.*/
    virtual QString name() const = 0;
    /** @return the parse job that is used by background parser to parse given @p url.*/
    virtual ParseJob *createParseJob(const KUrl &url) = 0;

};

}

KDEV_DECLARE_EXTENSION_INTERFACE( KDevelop, ILanguageSupport, "org.kdevelop.ILanguageSupport")
Q_DECLARE_INTERFACE( KDevelop::ILanguageSupport, "org.kdevelop.ILanguageSupport" )

#endif
