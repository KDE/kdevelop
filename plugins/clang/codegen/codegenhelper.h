/*
    SPDX-FileCopyrightText: 2007 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef CODEGENHELPER_H
#define CODEGENHELPER_H

#include <language/duchain/declaration.h>
#include <language/duchain/identifier.h>
#include <language/duchain/types/abstracttype.h>

struct Signature;

namespace CodegenHelper {
///Returns the type that should be used for shortened printing of the same.
KDevelop::AbstractType::Ptr typeForShortenedString(KDevelop::Declaration* decl);
///Returns a shortened string version of the type attached to the given declaration.
///@param desiredLength the desired length. No guarantee that the resulting string will be this short. With the default-value, no shortening will happen in most cases.
///@param ctx visibility context to consider. All prefixes of types are shortened to the minimum length while staying visible from here
///@param stripPrefix this prefix will be stripped from qualified identifiers. This is useful to remove parts of the current context.
QString shortenedTypeString(KDevelop::Declaration* decl, KDevelop::DUContext* ctx, int desiredLength = 10000,
                            const KDevelop::QualifiedIdentifier& stripPrefix = {});
QString shortenedTypeString(const KDevelop::AbstractType::Ptr& type, KDevelop::DUContext* ctx, int desiredLength = 10000,
                            const KDevelop::QualifiedIdentifier& stripPrefix = {});
KDevelop::IndexedTypeIdentifier shortenedTypeIdentifier(const KDevelop::AbstractType::Ptr& type, KDevelop::DUContext* ctx, int desiredLength = 10000,
                                                        const KDevelop::QualifiedIdentifier& stripPrefix = {});

///Returns a simplified string version of the given type: Template default-parameters are stripped away, qualified identifiers are simplified so they are as short as possible, while staying visible from the given context.
QString simplifiedTypeString(const KDevelop::AbstractType::Ptr& type, KDevelop::DUContext* visibilityFrom);

QString makeSignatureString(const KDevelop::Declaration* functionDecl, const Signature& signature, const bool editingDefinition);
}

#endif // CODEGENHELPER_H
