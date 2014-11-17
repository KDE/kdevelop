/*
 *    This file is part of KDevelop
 *
 *    Copyright 2013 Olivier de Gaalon <olivier.jg@gmail.com>
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Library General Public
 *    License as published by the Free Software Foundation; either
 *    version 2 of the License, or (at your option) any later version.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Library General Public License for more details.
 *
 *    You should have received a copy of the GNU Library General Public License
 *    along with this library; see the file COPYING.LIB.  If not, write to
 *    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *    Boston, MA 02110-1301, USA.
 */

#ifndef CLANGINDEX_H
#define CLANGINDEX_H

#include <duchain/clangduchainexport.h>

#include <util/path.h>

#include <QReadWriteLock>
#include <QSharedPointer>

#include <clang-c/Index.h>

class ClangParsingEnvironment;
class ClangPCH;

class KDEVCLANGDUCHAIN_EXPORT ClangIndex
{
public:
    ClangIndex();
    ~ClangIndex();

    CXIndex index() const;

    /**
     * @returns the existing ClangPCH for the @param pchInclude
     * The PCH is created using @param includePaths and @param defines if it doesn't exist
     * This function is thread safe.
     */
    QSharedPointer<const ClangPCH> pch(const ClangParsingEnvironment& defines);

private:
    CXIndex m_index;

    QReadWriteLock m_pchLock;
    QHash<KDevelop::Path, QSharedPointer<const ClangPCH>> m_pch;
};

#endif //CLANGINDEX_H
