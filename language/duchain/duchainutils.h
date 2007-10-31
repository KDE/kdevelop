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

#include <languageexport.h>

namespace KDevelop {

class Declaration;

namespace DUChainUtils {
  KDEVPLATFORMLANGUAGE_EXPORT KTextEditor::CodeCompletionModel::CompletionProperties completionProperties(Declaration* dec);
  KDEVPLATFORMLANGUAGE_EXPORT KIcon iconForProperties(KTextEditor::CodeCompletionModel::CompletionProperties p);
  KDEVPLATFORMLANGUAGE_EXPORT KIcon iconForDeclaration(Declaration* dec);
}

}

#endif // DUCHAINUTILS_H
