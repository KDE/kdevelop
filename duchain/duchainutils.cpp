/*
 * Copyright 2014  Kevin Funk <kfunk@kde.org>
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

#include "duchainutils.h"

#include "util/clangdebug.h"

#include <language/duchain/declaration.h>
#include <language/duchain/duchainutils.h>
#include <language/duchain/ducontext.h>
#include <language/duchain/functiondefinition.h>

#include "macrodefinition.h"
#include "clangducontext.h"
#include "clangparsingenvironmentfile.h"

using namespace KDevelop;

namespace ClangIntegration {

KTextEditor::Range DUChainUtils::functionSignatureRange(const Declaration* decl)
{
    if (!decl->isFunctionDeclaration()) {
        qCWarning(KDEV_CLANG) << "Invalid declaration:" << decl;
        return {};
    }

    auto functionContext = decl->internalContext();
    Q_ASSERT(functionContext);
    auto childContexts = functionContext->childContexts();
    if (childContexts.isEmpty()) {
        return functionContext->rangeInCurrentRevision();
    }

    const auto start = functionContext->rangeInCurrentRevision().start();
    const auto end = childContexts[0]->rangeInCurrentRevision().start();
    return {start, end};
}

void DUChainUtils::registerDUChainItems()
{
    duchainRegisterType<ClangTopDUContext>();
    duchainRegisterType<ClangParsingEnvironmentFile>();
    duchainRegisterType<ClangNormalDUContext>();
    duchainRegisterType<MacroDefinition>();
}

void DUChainUtils::unregisterDUChainItems()
{
    /// FIXME: this is currently not supported by the DUChain code...
    /// When the items are unregistered on plugin destruction, we'll get hit by
    /// assertions later on when the DUChain is finalized. There, when the data is getting cleaned up,
    /// we try to load all kinds of items again which would fail to find our items if we unregister.
    /// So let's not do it...
/*
    duchainUnregisterType<ClangTopDUContext>();
    duchainUnregisterType<ClangParsingEnvironmentFile>();
    duchainUnregisterType<ClangNormalDUContext>();
    duchainUnregisterType<MacroDefinition>();
*/
}

}
