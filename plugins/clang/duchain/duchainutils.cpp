/*
    SPDX-FileCopyrightText: 2014 Kevin Funk <kfunk@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "duchainutils.h"

#include "util/clangdebug.h"

#include <language/duchain/declaration.h>
#include <language/duchain/duchainutils.h>
#include <language/duchain/ducontext.h>
#include <language/duchain/functiondefinition.h>
#include <language/duchain/types/typeregister.h>

#include "macrodefinition.h"
#include "clangducontext.h"
#include "clangparsingenvironmentfile.h"
#include "types/classspecializationtype.h"

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

    TypeSystem::self().registerTypeClass<ClassSpecializationType, ClassSpecializationTypeData>();
}

void DUChainUtils::unregisterDUChainItems()
{
    TypeSystem::self().unregisterTypeClass<ClassSpecializationType, ClassSpecializationTypeData>();

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

ParseSessionData::Ptr DUChainUtils::findParseSessionData(const IndexedString &file, const IndexedString &tufile)
{
    DUChainReadLocker lock;
    auto context = KDevelop::DUChainUtils::standardContextForUrl(file.toUrl());
    if (!context || !context->ast()) {
        // no cached data found for the current file, but maybe
        // we are lucky and can grab it from the TU context
        // this happens e.g. when originally a .cpp file is open and then one
        // of its included files is opened in the editor.
        context = KDevelop::DUChainUtils::standardContextForUrl(tufile.toUrl());
    }

    if (context) {
        return ParseSessionData::Ptr(dynamic_cast<ParseSessionData*>(context->ast().data()));
    }
    return {};
}

}
