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

#ifndef TUDUCHAIN_H
#define TUDUCHAIN_H

#include <duchain/clangduchainexport.h>
#include "templatehelpers.h"
#include "cursorkindtraits.h"
#include "clanghelpers.h"
#include "clangducontext.h"
#include "macrodefinition.h"
#include "util/clangdebug.h"
#include "util/clangutils.h"
#include "util/clangtypes.h"

#include <util/pushvalue.h>

#include <language/duchain/duchainlock.h>
#include <language/duchain/classdeclaration.h>
#include <language/duchain/stringhelpers.h>

#include <language/duchain/types/pointertype.h>
#include <language/duchain/types/arraytype.h>
#include <language/duchain/types/referencetype.h>
#include <language/duchain/types/functiontype.h>
#include <language/duchain/types/structuretype.h>
#include <language/duchain/types/enumerationtype.h>
#include <language/duchain/types/enumeratortype.h>
#include <language/duchain/types/typealiastype.h>

#if CINDEX_VERSION_MINOR >= 25
#include <clang-c/Documentation.h>
#endif

#include <unordered_map>
#include <typeinfo>

namespace TUDUChain {
// TODO: Uh oh, this isn't nice. Can we make this better?
KDEVCLANGDUCHAIN_EXPORT void enableJSONTestRun();

KDEVCLANGDUCHAIN_EXPORT void visit(CXTranslationUnit tu, CXFile file,
                                   const IncludeFileContexts& includes, const bool update);

}

#endif //TUDUCHAIN_H
