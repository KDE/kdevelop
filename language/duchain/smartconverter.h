/*
 * Copyright 2006 Hamish Rodda <rodda@kde.org>
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

#ifndef DUCHAIN_SMARTCONVERTER_H
#define DUCHAIN_SMARTCONVERTER_H

#include "language/languageexport.h"

namespace KDevelop
{
class EditorIntegrator;
class ICodeHighlighting;
class DUContext;

class KDEVPLATFORMLANGUAGE_EXPORT SmartConverter
{
public:
  SmartConverter(KDevelop::EditorIntegrator* editor, KDevelop::ICodeHighlighting* hl = 0);
  ~SmartConverter();
  ///Converts all ranges in the given context to smart-ranges
  void convertDUChain(DUContext* context) const;
  void deconvertDUChain(DUContext* context) const;

private:
  class SmartConverterPrivate* const d;
};
}

#endif

// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on
