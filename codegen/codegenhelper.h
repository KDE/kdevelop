/*
   Copyright 2007 David Nolden <david.nolden.kdevelop@art-master.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef CODEGENHELPER_H
#define CODEGENHELPER_H

#include <language/duchain/declaration.h>
#include <language/duchain/identifier.h>
#include <language/duchain/types/abstracttype.h>

namespace CodegenHelper {
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
