/*
 * DUChain Utilities
 *
 * Copyright 2007 Hamish Rodda <rodda@kde.org>
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

#ifndef DUCHAINUTILS_H
#define DUCHAINUTILS_H

#include <KIcon>

#include <ktexteditor/codecompletionmodel.h>

#include "../languageexport.h"

class KUrl;

namespace KDevelop {

class Declaration;
class DUChainBase;
class SimpleCursor;
class HashedString;

namespace DUChainUtils {
  KDEVPLATFORMLANGUAGE_EXPORT KTextEditor::CodeCompletionModel::CompletionProperties completionProperties(Declaration* dec);
  KDEVPLATFORMLANGUAGE_EXPORT QIcon iconForProperties(KTextEditor::CodeCompletionModel::CompletionProperties p);
  KDEVPLATFORMLANGUAGE_EXPORT QIcon iconForDeclaration(Declaration* dec);
  /** Returns the Declaration/Definition/Use under the cursor, or zero. DUChain does not need to be locked. */
  KDEVPLATFORMLANGUAGE_EXPORT DUChainBase* itemUnderCursor(const KUrl& url, const SimpleCursor& cursor);
  KDEVPLATFORMLANGUAGE_EXPORT Declaration* declarationForItem(DUChainBase* item);
}

}

#endif // DUCHAINUTILS_H
