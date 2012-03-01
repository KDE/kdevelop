/* This file is part of KDevelop
    Copyright 2008 David Nolden <david.nolden.kdevelop@art-master.de>

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

#ifndef BUILTINOPERATORS_H
#define BUILTINOPERATORS_H

#include <language/duchain/types/abstracttype.h>

///The duchain must be locked!
///Returns the type that the builtin binary operator given through @param tokenKind with the given left and right operand types yields.
///When the types are constant integral types with values, the returned type will also have the value evaluated
///@return Zero if the given types can not be handled by a builtin operator
KDevelop::AbstractType::Ptr binaryOperatorReturnType(KDevelop::AbstractType::Ptr left, KDevelop::AbstractType::Ptr right, quint16 tokenKind);

#endif
