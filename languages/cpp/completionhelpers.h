/*
 * KDevelop C++ Code Completion Support
 *
 * Copyright 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef COMPLETIONHELPERS_H
#define COMPLETIONHELPERS_H

#include <duchain/typesystem.h>

class NormalDeclarationCompletionItem;
class QString;
class QVariant;
template<class T>
class QList;

namespace KDevelop {
  class Declaration;
  class DUContext;
}

void createArgumentList(const NormalDeclarationCompletionItem& item, QString& ret, QList<QVariant>* highlighting );

///Returns the type as which a declaration in the completion-list should be interpreted, which especially means that it returns the return-type of a function.
KDevelop::AbstractType::Ptr effectiveType( KDevelop::Declaration* decl );
KDevelop::DUContext* getArgumentContext(KDevelop::Declaration* decl);

#endif

