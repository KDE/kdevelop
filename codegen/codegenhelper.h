#ifndef CODEGENHELPER_H
#define CODEGENHELPER_H

#include <language/duchain/declaration.h>
#include <language/duchain/identifier.h>
#include <language/duchain/types/abstracttype.h>

namespace CodegenHelper
{

    ///Returns the type that should be used for shortened printing of the same.
KDevelop::AbstractType::Ptr typeForShortenedString(KDevelop::Declaration* decl);
///Returns a shortened string version of the type attached to the given declaration.
///@param desiredLength the desired length. No guarantee that the resulting string will be this short. With the default-value, no shortening will happen in most cases.
///@param ctx visibility context to consider. All prefixes of types are shortened to the minimum length while staying visible from here
///@param stripPrefix this prefix will be stripped from qualified identifiers. This is useful to remove parts of the current context.
QString shortenedTypeString(KDevelop::Declaration* decl, KDevelop::DUContext* ctx, int desiredLength = 10000, KDevelop::QualifiedIdentifier stripPrefix = KDevelop::QualifiedIdentifier());
QString shortenedTypeString(KDevelop::AbstractType::Ptr type, KDevelop::DUContext* ctx, int desiredLength = 10000, KDevelop::QualifiedIdentifier stripPrefix = KDevelop::QualifiedIdentifier());
KDevelop::IndexedTypeIdentifier shortenedTypeIdentifier(KDevelop::AbstractType::Ptr type, KDevelop::DUContext* ctx, int desiredLength = 10000, KDevelop::QualifiedIdentifier stripPrefix = KDevelop::QualifiedIdentifier());

///Returns a simplified string version of the given type: Template default-parameters are stripped away, qualified identifiers are simplified so they are as short as possible, while staying visible from the given context.
QString simplifiedTypeString(KDevelop::AbstractType::Ptr type, KDevelop::DUContext* visibilityFrom);
};

#endif // CODEGENHELPER_H
