/*
 * Copyright 2014  Milian Wolff <mail@milianw.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "includedfilecontexts.h"

#include "clangtypes.h"

#include <language/duchain/duchainlock.h>
#include <language/duchain/declaration.h>

using namespace KDevelop;

DeclarationPointer findDeclaration(CXCursor cursor, const IncludeFileContexts& includes)
{
    auto refLoc = clang_getCursorLocation(cursor);
    CXFile file = nullptr;
    clang_getFileLocation(refLoc, &file, nullptr, nullptr, nullptr);
    if (!file) {
        return {};
    }
    auto refCursor = CursorInRevision(ClangLocation(refLoc));

    Q_ASSERT(includes.contains(file));
    const auto& top = includes[file].topContext;

    DUChainReadLocker lock;
    Q_ASSERT(top);
    if (DUContext *local = top->findContextAt(refCursor)) {
        return DeclarationPointer(local->findDeclarationAt(refCursor));
    }
    return {};
}
