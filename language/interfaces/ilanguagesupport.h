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
#ifndef ILANGUAGESUPPORT_H
#define ILANGUAGESUPPORT_H

#include <kurl.h>

#include <interfaces/iextension.h>
#include "../languageexport.h"

namespace KDevelop {

class ParseJob;
class ILanguage;
class TopDUContext;

class KDEVPLATFORMLANGUAGE_EXPORT ILanguageSupport {
public:
    virtual ~ILanguageSupport() {}

    /** @return the name of the language.*/
    virtual QString name() const = 0;
    /** @return the parse job that is used by background parser to parse given @p url.*/
    virtual ParseJob *createParseJob(const KUrl &url) = 0;
    /** @return the language for this support.*/
    virtual ILanguage *language() = 0;
    /**
      * Only important for languages that can parse multiple different versions of a file, like C++ due to the preprocessor.
     * The default-implementation for other languages is "return DUChain::chainForDocument(url);"
     * 
     * @param allowProxyContext Whether proxy-contexts are allowed to be returned. In C++, a proxy-contexts has no direct content.
     *                          It mainly just imports an actual content-context, and represents multiple different versions
     *                          of the same content in the eyes of the preprocessor. Also, a proxy-context may contain the problem-
     *                          descriptions of preprocessor problems, so it should be used by anything that is interested in those
     *                          problems.
     *
     *  @warning The DUChain must be locked before calling this, @see KDevelop::DUChainReadLocker
     * 
     *  @return the standard context used by this language for the given @param url.
      * */
    virtual TopDUContext *standardContext(const KUrl& url, bool allowProxyContext = false);
};

}

KDEV_DECLARE_EXTENSION_INTERFACE_NS( KDevelop, ILanguageSupport, "org.kdevelop.ILanguageSupport")
Q_DECLARE_INTERFACE( KDevelop::ILanguageSupport, "org.kdevelop.ILanguageSupport")

#endif
