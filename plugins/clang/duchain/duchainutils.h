/*
    SPDX-FileCopyrightText: 2014 Kevin Funk <kfunk@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef DUCHAINUTILS_H
#define DUCHAINUTILS_H

#include "clangprivateexport.h"

#include "duchain/parsesession.h"

namespace KTextEditor {
class Range;
}

namespace KDevelop {
class Declaration;
}

namespace ClangIntegration {
namespace DUChainUtils
{

KDEVCLANGPRIVATE_EXPORT KTextEditor::Range functionSignatureRange(const KDevelop::Declaration* decl);

KDEVCLANGPRIVATE_EXPORT void registerDUChainItems();
KDEVCLANGPRIVATE_EXPORT void unregisterDUChainItems();

/**
 * Finds attached parse session data (aka AST) to the @p file
 *
 * If no session data found, then @p tuFile asked for the attached session data
 */
KDEVCLANGPRIVATE_EXPORT ParseSessionData::Ptr findParseSessionData(const KDevelop::IndexedString &file, const KDevelop::IndexedString &tufile);
}

}

#endif // DUCHAINUTILS_H
