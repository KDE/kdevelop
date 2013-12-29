/*
    This file is part of KDevelop

    Copyright 2013 Olivier de Gaalon <olivier.jg@gmail.com>
    Copyright 2013 Milian Wolff <mail@milianw.de>

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

#ifndef PARSESESSION_H
#define PARSESESSION_H

#include <QList>
#include <clang-c/Index.h>

#include <language/duchain/indexedstring.h>
#include <language/interfaces/iproblem.h>

#include "duchainexport.h"

class ClangIndex;

class KDEVCLANGDUCHAIN_EXPORT ParseSession
{
public:
    /**
     * @return a unique identifier for Clang documents.
     */
    static KDevelop::IndexedString languageString();

    /**
     * Parse the given @p contents.
     *
     * @param url The url for the document you want to parse.
     * @param contents The contents of the document you want to parse.
     */
    ParseSession(const KDevelop::IndexedString& url, const QByteArray& contents, ClangIndex* index);
    ~ParseSession();

    /**
     * @return the URL of this session
     */
    KDevelop::IndexedString url() const;

    QList<KDevelop::ProblemPointer> problems() const;

    CXTranslationUnit unit() const;

private:
    KDevelop::IndexedString m_url;
    QList<KDevelop::ProblemPointer> m_problems;
    CXTranslationUnit m_unit;
};

#endif // PARSESESSION_H
